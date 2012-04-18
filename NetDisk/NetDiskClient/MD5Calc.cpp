#include "StdAfx.h"
#include "MD5Calc.h"
#include "MD5Core.h"

MD5Calc::MD5Calc(void)
{
}


MD5Calc::~MD5Calc(void)
{
}

string MD5Calc::print_digest(unsigned char *p)
{
	int i;
	char buf[10];
	string strRet;

	for (i = 0; i < 16; ++i) {
		sprintf(buf,"%02x", *p++);
		strRet.append(buf);
	}

	return strRet;
}

//int MD5Calc::md5main(char * szFile, BOOL bCheck)
//{
//	int check = 1;
//	unsigned char digest[16];
//
//	if(bCheck) {
//
//		FILE * fp = fopen(szFile, "r");
//		do_check(fp);
//		fclose(fp);
//	} else {
//		FILE * fp = fopen(szFile, "rb");
//		if(!fp || mdfile(fp, digest)) {
//			AddItem(szFile,digest,digest,0);
//		} else {
//			AddItem(szFile,digest,digest,1);
//		}
//		if(fp)
//			fclose(fp);
//	}
//		
//	return 0;
//}

/*
int
__main(int argc, char **argv)
{
	int opt, rc = 0;
	int check = 0;
	FILE *fp;
	unsigned char digest[16];

	progname = *argv;
	while ((opt = pgp_getopt(argc, argv, "cbvp:h")) != EOF) {
		switch (opt) {
			case 'c': check = 1; break;
			case 'v': verbose = 1; break;
			case 'b': bin_mode = 1; break;
			default: usage();
		}
	}
	argc -= optind;
	argv += optind;
	if (check) {
		switch (argc) {
			case 0: fp = stdin; break;
			case 1: if ((fp = fopen(*argv, FOPRTXT)) == NULL) {
					perror(*argv);
					exit(2);
				}
				break;
			default: usage();
		}
		exit(do_check(fp));
	}
	if (argc == 0) {
		if (mdfile(stdin, digest)) {
			fprintf(stderr, "%s: read error on stdin\n", progname);
			exit(2);
		}
		print_digest(digest);
		printf("\n");
		exit(0);
	}
	for ( ; argc > 0; --argc, ++argv) {
		if (bin_mode)
			fp = fopen(*argv, FOPRBIN);
		else
			fp = fopen(*argv, FOPRTXT);
		if (fp == NULL) {
			perror(*argv);
			rc = 2;
			continue;
		}
		if (mdfile(fp, digest)) {
			fprintf(stderr, "%s: error reading %s\n", progname, *argv);
			rc = 2;
		} else {
			print_digest(digest);
			printf(" %c%s\n", bin_mode ? '*' : ' ', *argv);
		}
		fclose(fp);
	}
	exit(rc);
    return 0;
}
*/
int MD5Calc::mdfile(char *szFile, unsigned char *digest)
{
	FILE* fp = fopen(szFile,"rb");
	return mdfile(fp,digest);
}

int MD5Calc::mdfile(FILE *fp, unsigned char *digest)
{
	unsigned char buf[1024];
	MD5_CTX ctx;
	int n;

	MD5Init(&ctx);
	while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
		MD5Update(&ctx, buf, n);
	MD5Final(digest, &ctx);
	if (ferror(fp))
		return -1;
	return 0;
}

int MD5Calc::hex_digit(int c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return -1;
}

int MD5Calc::get_md5_line(FILE *fp, unsigned char *digest, char *file)
{
	char buf[1024];
	int i, d1, d2, rc;
	char *p = buf;

	if (fgets(buf, sizeof(buf), fp) == NULL)
		return -1;

	for (i = 0; i < 16; ++i) {
		if ((d1 = hex_digit(*p++)) == -1)
			return 0;
		if ((d2 = hex_digit(*p++)) == -1)
			return 0;
		*digest++ = d1*16 + d2;
	}
	if (*p++ != ' ')
		return 0;
	/*
	 * next char is an attribute char, space means text file
	 * if it's a '*' the file should be checked in binary mode.
	 */
	if (*p == ' ')
		rc = 1;
	else if (*p == '*')
		rc = 2;
	else {
		fprintf(stderr, "%s: unrecognized line: %s", "md5sum", buf);
		return 0;
	}
	++p;
	i = strlen(p);
	if (i < 2 || i > 255)
		return 0;
	p[i-1] = '\0';
	strcpy(file, p);
	return rc;
}

//int MD5Calc::do_check(FILE *chkf)
//{
//	int rc, ex = 0, failed = 0, checked = 0;
//	unsigned char chk_digest[16], file_digest[16];
//	char filename[256];
//	FILE *fp;
//	unsigned int flen = 14;
//
//	while ((rc = get_md5_line(chkf, chk_digest, filename)) >= 0) {
//		if (rc == 0)	/* not an md5 line */
//			continue;
//		if(filename[strlen(filename)-1] == '\r' ||
//			filename[strlen(filename)-1] == '\n')
//			filename[strlen(filename)-1] = 0;
//		
//		if (verbose) {
//			if (strlen(filename) > flen)
//				flen = strlen(filename);
//			fprintf(stderr, "%-*s ", flen, filename);
//		}
//		if (bin_mode || rc == 2)
//			fp = fopen(filename, FOPRBIN);
//		else
//			fp = fopen(filename, FOPRTXT);
//		if (fp == NULL) {
//			AddItem(filename,chk_digest,file_digest,2);
//			fprintf(stderr, "%s: can't open %s\n", progname, filename);
//			ex = 2;
//			continue;
//		}
//		if (mdfile(fp, file_digest)) {
//			AddItem(filename,chk_digest,file_digest,2);
//			fprintf(stderr, "%s: error reading %s\n", progname, filename);
//			ex = 2;
//			fclose(fp);
//			continue;
//		}
//		fclose(fp);
//		if (memcmp(chk_digest, file_digest, 16) != 0) {
//			AddItem(filename,chk_digest,file_digest, 0);
//			if (verbose)
//				fprintf(stderr, "FAILED\n");
//			else
//				fprintf(stderr, "%s: MD5 check failed for '%s'\n", progname, filename);
//			++failed;
//		} else {
//			if (verbose)
//				fprintf(stderr, "OK\n");
//			AddItem(filename,chk_digest,file_digest,1);
//		}
//
//		++checked;
//	}
//	if (verbose && failed)
//		fprintf(stderr, "%s: %d of %d file(s) failed MD5 check\n", progname, failed, checked);
//	if (!checked) {
//		fprintf(stderr, "%s: no files checked\n", progname);
//		return 3;
//	}
//	if (!ex && failed)
//		ex = 1;
//	return ex;
//}

/*
void MD5Calc::OnSize(UINT nType, int cx, int cy) 
{
	static bFirst;
	static offset_left;
	static offset_right;
	static offset_bottom;
	static offset_top;

	CDialog::OnSize(nType, cx, cy);
	
	if(m_List.GetSafeHwnd()) {
		
		if(!bFirst) {
			CRect parent;
			GetWindowRect(parent);
			CRect cr;
			m_List.GetWindowRect(cr);

			offset_left = cr.left - parent.left-4;
			offset_right = parent.right - cr.right-4;
			offset_top = cr.top - parent.top-22;
			offset_bottom = parent.bottom - cr.bottom;
			bFirst = TRUE;
		}
		m_List.MoveWindow(offset_left,offset_top,cx-offset_left - offset_right,
			offset_bottom - offset_top + cy - offset_bottom-12,TRUE);
	}
	
}
*/
