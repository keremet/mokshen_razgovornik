#define _GNU_SOURCE
#define _FILE_OFFSET_BITS 64

#include <dirent.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <glib.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
//FIXME free memory
//FIXME replace with g_build_filename
#define strconcat(str1, ...) g_strconcat(str1, __VA_ARGS__, NULL)

static void cp(const char* from, const char* to) {
	int fd_in = open(from, O_RDONLY);
	if (fd_in < 0)
		return;

	struct stat  stat;
	if (fstat(fd_in, &stat) < 0)
		return;

	int fd_out = open(to, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd_in < 0)
		return;

	off_t ret;
	do {
		ret = copy_file_range(fd_in, NULL, fd_out, NULL, stat.st_size, 0);
		if (fd_in < 0)
			return;

		stat.st_size -= ret;
	} while (stat.st_size > 0 && ret > 0);

	close(fd_in);
	close(fd_out);
}

static char* removeQuestionMark(const char *str) {
	size_t len = strlen(str);
	char *ret = malloc(len+1);
	int ret_len = 0;
	for (int i = 0; i < len; i++) {
		if ('?' == str[i])
			continue;
		ret[ret_len++] = str[i];
	}

	ret[ret_len] = 0; 	
	return ret;
}

static void addAudio(FILE* f, const char* srcDir, const char* dstDir) {
	struct dirent **de;
	int n = scandir(srcDir, &de, NULL, alphasort);
	for (int i = 0; i < n; i++) {
		if ('.' == de[i]->d_name[0] || de[i]->d_type != DT_REG)
			continue;

		gchar** arr = g_strsplit(de[i]->d_name, "___", 2);
		if (g_strv_length(arr) != 2)
			continue;

		if (!g_str_has_suffix(arr[1], ".wav"))
			continue;

		arr[1][strlen(arr[1]) - 4] = 0;
		const char *dstFN = removeQuestionMark(de[i]->d_name);
		cp(strconcat(srcDir, "/", de[i]->d_name), strconcat(dstDir, "/", dstFN));
		fprintf(f, "<tr><td>%s<td>%s<td><audio controls preload=none><source src=\"%s\" type=\"audio/wav\"></audio>", arr[0], arr[1], dstFN);
	}
}

static void processDir(const char* dataDir, const char* outputDir, const char* dirName) {
	const char* dstDir = strconcat(outputDir, "/", dirName);
	mkdir(dstDir, S_IRWXU | S_IRWXG | S_IRWXO);
	FILE* f = fopen(strconcat(dstDir, "/index.html"), "w");
	fprintf(f,
		"<html>"
		"<head>"
		"<title>Мокшень кяль</title>"
		"<meta charset=\"utf-8\">"
		"<style>"
		"p {font-size: 40pt;}"
		"h1 {font-size: 50pt;}"
		"h2 {font-size: 45pt;}"
		"td {font-size: 35pt;}"
		"</style>"
		"</head>"
		"<body>"
		"<p><a href=\"../index.html\">Обратно</a>"
		"<h1>%s</h1>"
		"<h2>Cлова"
		"<table border=1>"
		"<tr><td><b>По-русски<td><b>Мокшекс<td><b>Произношение", dirName);

	const char* dirNameFull = strconcat(dataDir, "/", dirName);
	addAudio(f, strconcat(dirNameFull, "/Слова"), dstDir);

	fputs("</table>"
		"<h2>Фразы"
		"<table border=1>"
		"<tr><td><b>По-русски<td><b>Мокшекс<td><b>Произношение", f);
	addAudio(f, strconcat(dirNameFull, "/Фразы"), dstDir);

	fputs("</table></body></html>", f);
	fclose(f);
}

int main(int argc, char** argv) {
	if (argc != 1 + 2) {
		fprintf(stderr, "Usage: %s <data dir> <output dir>\n", argv[0]);
		return 1;
	}

	const char* dataDir = argv[1];
	const char* outputDir = argv[2];

	FILE* fMain = fopen(strconcat(outputDir, "/index.html"), "w");
	fputs(
		"<html>"
		"<head>"
		"<title>Мокшень кяль</title>"
		"<meta charset=\"utf-8\">"
		"<style>p {font-size: 40pt;}</style>"
		"</head>"
		"<body>", 
		fMain);
	struct dirent **de;
	int n = scandir(dataDir, &de, NULL, alphasort);
	for (int i = 0; i < n; i++) {
		if ('.' == de[i]->d_name[0] || de[i]->d_type != DT_DIR)
			continue;
		fprintf(fMain, "<p><a href=\"%s/index.html\">%s</a>", de[i]->d_name, de[i]->d_name);
		processDir(dataDir, outputDir, de[i]->d_name);
	}
	fputs("</body></html>", fMain);
	fclose(fMain);
	return 0;
}
