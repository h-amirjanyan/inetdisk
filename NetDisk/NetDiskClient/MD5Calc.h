#pragma once
#include <iostream>
using namespace std;

#define	FOPRTXT	"r"
#define	FOPRBIN	"rb"

class MD5Calc
{
public:
	MD5Calc(void);
	~MD5Calc(void);

	//int do_check(FILE *chkf);
	int mdfile(char* szFile,unsigned char *digest);
	int mdfile(FILE *fp, unsigned char *digest);
	int get_md5_line(FILE *fp, unsigned char *digest, char *file);
	int hex_digit(int c);
	string print_digest(unsigned char *p);
	//int md5main(char * szFile, BOOL bCheck);
};

