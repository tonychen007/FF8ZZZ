#pragma once

#include "FF8.h"
#include "LZS.h"
#include "QLZ4.h"

FF8::FF8(const char* src, const char* dst) {
	fs::path file(src);
	if (!fs::exists(file)) {
		fprintf(stderr, "file not existed\n");
		exit(-1);
	}
	if (fs::exists(dst)) {
		fs::remove_all(dst);
	}
	fs::create_directory(dst);
	fs::path dstpath(dst);
	srcpath_ = file;
	dstpath_ = dstpath;
}

void FF8::dumpZZZ() {
	getZZZFileData();

	FILE* fr = fopen(srcpath_.generic_string().c_str(), "rb");
	for (FileData& f : files_) {
		char buf[255] = { '\0' };
		strcpy(buf, dstpath_.generic_string().c_str());
		strcat(buf, "\\");
		strcat(buf, f.filename_);
		strcat(buf, "\0");

		fs::path dirPath(buf);
		fs::create_directories(dirPath.parent_path());
		FILE* fw = fopen(buf, "wb");
		unique_ptr<char[]> uchar(new char[f.filesize_]);
		fseek(fr, f.fileoffset_, SEEK_SET);
		fread(uchar.get(), sizeof(char), f.filesize_, fr);
		fwrite(uchar.get(), sizeof(char), f.filesize_, fw);
		fclose(fw);
		printf("written file:%s\n", f.filename_);
	}
	fclose(fr);
}

void FF8::getZZZFileData() {
	FILE* fp = fopen(srcpath_.generic_string().c_str(), "rb");
	fread((void*)&filenum_, sizeof(int), 1, fp);

	for (int i = 0; i < filenum_; i++) {
		//read filename
		//read offset
		FileData fdata;
		int len;
		fread((void*)&len, sizeof(int), 1, fp);
		fread(fdata.filename_, sizeof(char), len, fp);
		fread((void*)&fdata.fileoffset_, sizeof(__int64), 1, fp);
		fread((void*)&fdata.filesize_, sizeof(int), 1, fp);
		files_.emplace_back(std::move(fdata));
	}

	fclose(fp);
}

void FF8::dumpFS(const char* path, const char* dst, int isSub) {
	getFsFileInfo(path);
	static int isBegin = 1;

	// dump fs file
	FILE* fr = fopen(path, "rb");
	if (!isSub || isBegin) {
		fs::remove_all(dst);
		fs::create_directory(dst);
		isBegin = 0;
	}
	for (FSData& fs : fss_) {
		string dstfile = dst + string(fs.filename_);
		fs::path dirPath(dstfile);
		fs::create_directories(dirPath.parent_path());

		// check if compress, recalc size
		int len;
		const char* data = getCompressedData(fr,fs,&len);
		FILE* fw = fopen(dstfile.data(), "wb");
		fwrite(data, sizeof(char), len, fw);
		fclose(fw);
		printf("Written file:%s\n", dstfile.data());
		
		if (isSub) {
			if (dstfile.rfind(".fs") == dstfile.length() - 3)
				recurfs_.push_back(dstfile);
		}		
	}
	fss_.clear();
	fclose(fr);

	while (!recurfs_.empty()) {
		string fsnm = recurfs_.back();
		recurfs_.pop_back();
		printf("*****recuring fs*****\n");
		dumpFS(fsnm.data(),dst,1);
	}	
}

void FF8::getFsFileInfo(const char* path) {
	// read fl //name
	// read fi //size offset
	// read fs
	if (!fs::exists(path)) {
		fprintf(stderr, "%s not existed\n", path);
		exit(-1);
	}

	string prepath = string(path).substr(0, strlen(path) - 1);
	string fipath = prepath + "i";
	string flpath = prepath + "l";
	string filename;
	int dummpy;

	if (!fs::exists(fipath)) {
		fprintf(stderr, "%s not existed\n", fipath.data());
		exit(-1);
	}
	else if (!fs::exists(flpath)) {
		fprintf(stderr, "%s not existed\n", flpath.data());
		exit(-1);
	}

	ifstream flin(flpath);
	FILE* fiin = fopen(fipath.data(), "rb");
	while (getline(flin, filename)) {
		FSData fs;
		filename = filename.substr(strlen("c:\\ff8\\data\\"));
		memcpy(fs.filename_, filename.data(), filename.size());
		// int32 size
		// int32 pos
		// int32 lzs compression
		fread(&fs.size_, sizeof(int), 1, fiin);
		fread(&fs.offset_, sizeof(int), 1, fiin);
		fread(&fs.compress_, sizeof(int), 1, fiin);
		fss_.emplace_back(std::move(fs));

		/*
		if (filename.rfind(".fs") == filename.size() - 3) {
			recurfs_.push_back(filename);
		}
		*/
	}
	flin.close();
	fclose(fiin);
}

// The fucking code
const char* FF8::getCompressedData(FILE* fp, const FSData& fs, int* len) {
	int uncompressSize = fs.size_;
	int compressLevel = fs.compress_;
	int offset = fs.offset_;
	
	fseek(fp, offset, SEEK_SET);
	if (isCompressed(compressLevel)) {
		int compressedSize = 0;

		fseek(fp, offset, SEEK_SET);
		int rd = fread(&compressedSize, sizeof(int), 1, fp);
		if ( rd != 1) return nullptr;
		if (compressedSize > uncompressSize * 2) return nullptr;
		
		char* buff = new char[compressedSize];
		if (fread(buff, sizeof(char), compressedSize, fp) != compressedSize) return nullptr;

		const char* data = decompress(buff, compressLevel,compressedSize, uncompressSize, len);
		return data;
	}

	*len = uncompressSize;
	char* buff = new char[uncompressSize];
	fread(buff, sizeof(char), uncompressSize, fp);
	return buff;
}

bool FF8::isCompressed(int compressLevel) {
	return (compressLevel == (int)ECompressLevel::ECompressionLzs || compressLevel == (int)ECompressLevel::ECompressionLz4);
}

const char* FF8::decompress(const char* buff, int compressLevel, int compressedSize, int uncompressedSize, int* len) {
	if (compressLevel == (int)ECompressLevel::ECompressionLz4) {
		auto ret = QLZ4::decompress(buff, compressedSize, uncompressedSize);
		*len = ret.length();
		return ret.constData();
	}

	auto ret = LZS::decompress(buff, compressedSize, uncompressedSize);
	*len = ret.length();
	return ret.constData();
}