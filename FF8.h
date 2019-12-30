#pragma once

#include<fstream>
#include<string>
#include<vector>
#include<filesystem>
using namespace std;
namespace fs = std::filesystem;

class FF8 {
public:
	FF8() {}
	FF8(const char* src, const char* dst);
	void dumpZZZ();
	void dumpFS(const char* path, const char* dst, int isSub = 0);

private:
	struct FSData;

	void getZZZFileData();
	void getFsFileInfo(const char* path);

	const char* getCompressedData(FILE* fp, const FSData& fs, int* len);
	bool isCompressed(int compLevel);
	const char* decompress(const char* buff, int compressLevel, int compressedSize, int uncompressedSize, int* len);
private:
	struct FileData;
	struct FSData;

	enum class ECompressLevel {
		ECompressionNone,
		ECompressionLzs,
		ECompressionLz4,
		ECompressionUnknown,

		EMAX_CompressLevel
	};

	fs::path srcpath_;
	fs::path dstpath_;
	int filenum_;
	vector<FileData> files_;
	vector<FSData> fss_;
	vector<string> recurfs_;
};

struct FF8::FileData {
	char filename_[256] = { '\0' };
	int fileoffset_;
	int filesize_;
};

struct FF8::FSData {
	char filename_[256] = { '\0' };
	int size_;
	int offset_;
	int compress_;
};