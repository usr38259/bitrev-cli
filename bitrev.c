
#include <stdio.h>
#ifdef __linux__
#include <sys/stat.h>
#else
#include <io.h>
#endif

#define MAXFILESIZE	16777216

#ifdef _WIN32
#ifndef _CRTAPI1
#define _CRTAPI1 __cdecl
#endif
#else
#define _CRTAPI1
#endif

const char *in_fname  = NULL;
const char *out_fname = NULL;

long file_length (FILE *f);
void print_usage (void);
void usage (void);

char btable [256];

int _CRTAPI1 main (int argc, const char* argv [])
{
	int i, c, cr = 0;
	const char *fname, *pa;
	FILE *fin, *fout = NULL;
	char f_e = 0;
	long flen;

	for (i = 1; i < argc; i++) {
		pa = argv [i];
		if (in_fname == NULL) in_fname = pa;
		else if (out_fname == NULL) out_fname = pa;
		else { f_e = 1; break; }
	}
	if (f_e || out_fname == NULL) { puts ("Command line error"); usage (); return 1; }
	if (in_fname == NULL) {
		print_usage (); return 0;
	}
	for (i = 0; i < 256; i++)
		btable [i] = i << 7 | i << 5 & 0x40 | i << 3 & 0x20 | i << 1 & 0x10 |
			i >> 1 & 0x08 | i >> 3 & 0x04 | i >> 5 & 0x02 | i >> 7 & 0x01;

	fin = fopen (in_fname, "rb");
	if (fin == NULL) {
foerr:		printf ("File '%s' open error\n", in_fname);
		goto ioerr;
	}
	flen = file_length (fin);
	if (flen == -1) {
iomsg:		printf ("%s: file I/O error: ", fname);
ioerr:		perror (NULL); cr = 1;
		goto err;
	}
	if (MAXFILESIZE && flen > MAXFILESIZE || flen < 0)
		{ puts ("File is too large"); cr = 1; goto err; }
	if (flen == 0) { puts ("File is empty"); cr = 0; goto err; }
	fout = fopen (out_fname, "wb");
	fname = out_fname;
	if (fout == NULL) goto foerr;
	for (i = 0; i < flen; i++) {
		c = fgetc (fin);
		if (c <= EOF) if (ferror (fin))
			{ fname = in_fname; goto iomsg; }
		else { cr = 1; goto err; }
		if (c >= 256) { perror ("Internal error"); cr = 1; goto err; }
		cr = fputc (btable [c], fout);
		if (cr <= EOF) if (ferror (fout))
			{ fname = out_fname; goto iomsg; }
		else { cr = 1; goto err; }
	}
err:	if (fout) fclose (fout);
	if (fin) fclose (fin);
	return cr;
}

void print_usage (void)
{
	puts ("\nReverse bit order of bytes in binary file"
		" (no byte sequence swap or change)");
	usage ();
}

void usage (void)
{
	puts ("Usage:\n  bitrev <input file> <output file>"
#ifdef __linux__
	"\n"
#endif
);
}

long file_length (FILE *f)
{
#ifdef __linux__
	struct stat st;
	if (fstat (fileno (f), &st) != 0)
		return -1;
	return st.st_size;
#else
	return filelength (fileno (f));
#endif
}
