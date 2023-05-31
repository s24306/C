#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <jpeglib.h>
#include <argtable2.h>
#include <string.h>

const char * input_file;
const char * output_file;
double times = 1;

#define MAX_COLOR_VALUE 255


char ASCIItable[13] = {'@', '#', '0', 'o', '*', '"', '.', ' '};

/* we will be using this uninitialized pointer later to store raw, uncompressd image */
JSAMPARRAY row_pointers = NULL;


/* dimensions of the image we want to write */
JDIMENSION width;
JDIMENSION height;
int num_components;
int quality = 75;
J_COLOR_SPACE color_space;
void write_file(const char *filename, char str[(int)(width/times)][((int)(height/times))]);

char getASCII(float f){
	return ASCIItable[(int)round(f*7)];
}

void toASCII(){
  	if (color_space != JCS_RGB)  return;

	width = (int)(((int)(width/times)) * times);
	height = (int)(((int)(height/times)) * times);

	char ASCIIarray[(int)(width/times)][((int)(height/times))+1];

	for(int y = 0; y < height;){
		for(int x = 0; x < width;){
			double ASCIIcharValue = 0;
			for(int i = 0; i < times; i++){
				for(int j = 0; j < times; j++){
					JSAMPROW row = row_pointers[y+i];
					JSAMPROW row_ptr = &(row[(x+j)*3]);
					ASCIIcharValue += row_ptr[0] + row_ptr[1] + row_ptr[2];
				}
			}
			double squareIntensity = ASCIIcharValue/(MAX_COLOR_VALUE*3*times*times);
			ASCIIarray[(int)(y/times)][(int)(x/times)] = getASCII(squareIntensity);
			x += times;
		}
		ASCIIarray[(int)(y/times)][(int)(width/times)] = '\n';
		y += times;
	}

	for (int y=0; y<width; y++){
		free(row_pointers[y]);
	}
	free(row_pointers);
	write_file(output_file, ASCIIarray);
}


void process_file(){
		toASCII();
}


void abort_(const char * s, ...)
{
	va_list args;
	va_start(args, s);
	vfprintf(stderr, s, args);
	fprintf(stderr, "\n");
	va_end(args);
	abort();
}

void write_file(const char *filename, char str[(int)(width/times)][((int)(height/times))]){
	FILE *outfile = fopen( filename, "at" );
	if ( !outfile ){
		abort_("Error opening output text file %s!\n", filename);
	}

	char *ptr;
	ptr = &str[0][0];

	fwrite(ptr, sizeof(char), (int)(width/times) * ((int)(height/times)) + (int)(width/times), outfile);

	fclose(outfile);
}
/**
 * read_jpeg_file Reads from a jpeg file on disk specified by filename and saves into the 
 * raw_image buffer in an uncompressed format.
 * 
 * \returns positive integer if successful, -1 otherwise
 * \param *filename char string specifying the file name to read from
 *
 */

void read_jpeg_file( const char *filename )
{
	/* these are standard libjpeg structures for reading(decompression) */
	struct jpeg_decompress_struct cinfo;
	struct jpeg_error_mgr jerr;
	/* libjpeg data structure for storing one row_left_upper, that is, scanline of an image */
	int y;
	
	FILE *infile = fopen( filename, "rb" );
	
	if ( !infile )
	{
		abort_("Error opening input jpeg file %s!\n", filename);
	}
	/* here we set up the standard libjpeg error handler */
	cinfo.err = jpeg_std_error( &jerr );
	jpeg_create_decompress( &cinfo );
	/* this makes the library read from infile */
	jpeg_stdio_src( &cinfo, infile );
	/* reading the image header which contains image information */
	jpeg_read_header( &cinfo, TRUE );


	/* Start decompression jpeg here */
	jpeg_start_decompress( &cinfo );
	width = cinfo.output_width;
	height = cinfo.output_height;
	num_components = cinfo.out_color_components;
	color_space = cinfo.out_color_space;
	
	/* allocate memory to hold the uncompressed image */
	size_t rowbytes = width * num_components;
	row_pointers = (JSAMPARRAY) malloc(sizeof(j_common_ptr) * height);
	for (y=0; y<height; y++){
		row_pointers[y] = (JSAMPROW) malloc(rowbytes);
	}

	
	/* read one scan line at a time */
	y=0;
	JSAMPARRAY tmp = row_pointers;
	while( cinfo.output_scanline < cinfo.image_height )
	{
		y = jpeg_read_scanlines( &cinfo, tmp, 1 );
		tmp +=y;
	}
	/* wrap up decompression, destroy objects, free pointers and close open files */
	jpeg_finish_decompress( &cinfo );
	jpeg_destroy_decompress( &cinfo );
// 	free( row_pointer[0] );
	fclose( infile );
	/* yup, we succeeded! */
}


int main(int argc, char **argv){   
  // Options
  struct arg_file *input_file_arg = arg_file1("i", "input-file", "<input>", "Input JPEG File");
  struct arg_file *output_file_arg = arg_file1("o", "out-file" , "<output>", "Output txt File");
  struct arg_dbl *times_arg = arg_dbl0("t", "times" , "<times>", "Multiplyer");
  struct arg_lit *help = arg_lit0("h","help", "print this help and exit");
  struct arg_end *end = arg_end(10);
  
  int nerrors;
  
  void *argtable[] = {input_file_arg, output_file_arg, times_arg, help, end};
  
  if (arg_nullcheck(argtable) != 0) printf("error: insufficient memory\n");
  
  times_arg->dval[0] = 1;

  nerrors = arg_parse(argc, argv, argtable);
  
  if (help->count > 0){
     printf("Usage: geometry");
     arg_print_syntax(stdout, argtable,"\n");
     arg_print_glossary(stdout, argtable,"  %-25s %s\n");
     arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
     return 0;
  }

  if (nerrors==0){
     input_file = input_file_arg->filename[0];
     output_file = output_file_arg->filename[0];
	 times = times_arg->dval[0];
  }
  else{
     arg_print_errors(stderr, end, "point");
     arg_print_glossary(stderr, argtable, " %-25s %s\n");
     arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
     return 1;
  }

   read_jpeg_file(input_file);
   process_file();
   arg_freetable(argtable, sizeof(argtable)/sizeof(argtable[0]));
   return 0;
}

