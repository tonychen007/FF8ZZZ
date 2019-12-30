#include<stdio.h>
#include<fstream>
#include<string>
#include<vector>
#include<filesystem>

#include"FF8.h"
#include<QtCore/QtCore>

using namespace std;
namespace fs = std::filesystem;

void dumpZZZ(const char* src, const char* dst) {
	if (src == nullptr || dst == nullptr) {
		printf("usgae: ff8 <zzzfile> <dstDir>\n");
		exit(-1);
	}

	//FF8 ff8("z:\\main.zzz", "z:\\FF81");
	FF8 ff8(src, dst);
	ff8.dumpZZZ();
}

void dumpFS(const char* path, const char* dst, int isSub = 0) {
	if (path == nullptr) {
		printf("usgae: ff8 <fsfile path> <dst path> <isSub>\n");
		exit(-1);
	}
	FF8 ff8;
	ff8.dumpFS(path, dst, isSub);
}

int main(int argc, char** argv) {
	// TODO boost command option
	dumpFS(".\\fs\\field.fs","z:\\tmp\\", 0);

	return 0;
}