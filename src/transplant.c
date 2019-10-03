#include "const.h"
#include "transplant.h"
#include "debug.h"

#ifdef _STRING_H
#error "Do not #include <string.h>. You will get a ZERO."
#endif

#ifdef _STRINGS_H
#error "Do not #include <strings.h>. You will get a ZERO."
#endif

#ifdef _CTYPE_H
#error "Do not #include <ctype.h>. You will get a ZERO."
#endif

int cmp(char *one, char *two); 
static char *sbytes;
static int slength;
static long num;
int intToBigEndianHex(int val, int byteSize);
int strLen(char *str);
int charStr(char *str);
int header(int type);
int checkSequence(int type, int depth);
int BigEndianToInt(int numBytes);
int entrySize;

/*
 * You may modify this file and/or move the functions contained here
 * to other source files (except for main.c) as you wish.
 *
 * IMPORTANT: You MAY NOT use any array brackets (i.e. [ and ]) and
 * you MAY NOT declare any arrays or allocate any storage with malloc().
 * The purpose of this restriction is to force you to use pointers.
 * Variables to hold the pathname of the current file or directory
 * as well as other data have been pre-declared for you in const.h.
 * You must use those variables, rather than declaring your own.
 * IF YOU VIOLATE THIS RESTRICTION, YOU WILL GET A ZERO!
 *
 * IMPORTANT: You MAY NOT use floating point arithmetic or declare
 * any "float" or "double" variables.  IF YOU VIOLATE THIS RESTRICTION,
 * YOU WILL GET A ZERO!
 */

/*
 * A function that returns printable names for the record types, for use in
 * generating debugging printout.
 */
static char *record_type_name(int i) {
    switch(i) {
    case START_OF_TRANSMISSION:
	return "START_OF_TRANSMISSION";
    case END_OF_TRANSMISSION:
	return "END_OF_TRANSMISSION";
    case START_OF_DIRECTORY:
	return "START_OF_DIRECTORY";
    case END_OF_DIRECTORY:
	return "END_OF_DIRECTORY";
    case DIRECTORY_ENTRY:
	return "DIRECTORY_ENTRY";
    case FILE_DATA:
	return "FILE_DATA";
    default:
	return "UNKNOWN";
    }
}

/*
 * @brief  Initialize path_buf to a specified base path.
 * @details  This function copies its null-terminated argument string into
 * path_buf, including its terminating null byte.
 * The function fails if the argument string, including the terminating
 * null byte, is longer than the size of path_buf.  The path_length variable 
 * is set to the length of the string in path_buf, not including the terminating
 * null byte.
 *
 * @param  Pathname to be copied into path_buf.
 * @return 0 on success, -1 in case of error
 */
int path_init(char *name) {
    int length = 0; 
    while(*name != '\0'){    
        *(path_buf+length) = *(name);
        name++; 
        length++;
        if(length > PATH_MAX)
            return -1;        
    }
    *(path_buf+length) = *(name);
    path_length = length;
    return 0;
}

/*
 * @brief  Append an additional component to the end of the pathname in path_buf.
 * @details  This function assumes that path_buf has been initialized to a valid
 * string.  It appends to the existing string the path separator character '/',
 * followed by the string given as argument, including its terminating null byte.
 * The length of the new string, including the terminating null byte, must be
 * no more than the size of path_buf.  The variable path_length is updated to
 * remain consistent with the length of the string in path_buf.
 * 
 * @param  The string to be appended to the path in path_buf.  The string must
 * not contain any occurrences of the path separator character '/'.
 * @return 0 in case of success, -1 otherwise.
 */
int path_push(char *name) {
    int length = path_length; 
    *(path_buf+length) = '/';
    length++;
    while(*name != '\0'){
        if(*name == '/'){
            printf("no / in push path\n");
            return -1;    
        }
        *(path_buf+length) = *(name);
        name++; 
        length++;
        if(length > PATH_MAX)
            return -1;        
    }
    *(path_buf+length) = *(name);
    path_length = length;
    return 0;
}

/*
 * @brief  Remove the last component from the end of the pathname.
 * @details  This function assumes that path_buf contains a non-empty string.
 * It removes the suffix of this string that starts at the last occurrence
 * of the path separator character '/'.  If there is no such occurrence,
 * then the entire string is removed, leaving an empty string in path_buf.
 * The variable path_length is updated to remain consistent with the length
 * of the string in path_buf.  The function fails if path_buf is originally
 * empty, so that there is no path component to be removed.
 *
 * @return 0 in case of success, -1 otherwise.
 */
int path_pop() {
    if(path_length == 0)
        return -1;  
    int i = path_length;
    int removed = 0;
    while(i != 0){
        if(*(path_buf+i) == '/'){
            *(path_buf+i) = '\0';
            path_length = i;
            removed = 1;
            return 0;
        }
        i--;
    }
    if(removed == 0){
        path_length = 0;
        *path_buf = '\0';
    }
    return 0;
}

/*
 * @brief Deserialize directory contents into an existing directory.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory.  It reads (from the standard input) a sequence of DIRECTORY_ENTRY
 * records bracketed by a START_OF_DIRECTORY and END_OF_DIRECTORY record at the
 * same depth and it recreates the entries, leaving the deserialized files and
 * directories within the directory named by path_buf.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * each of the records processed.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including depth fields in the records read that do not match the
 * expected value, the records to be processed to not being with START_OF_DIRECTORY
 * or end with END_OF_DIRECTORY, or an I/O error occurs either while reading
 * the records from the standard input or in creating deserialized files and
 * directories.
 */
int deserialize_directory(int depth) {
    // To be implemented.
    //check for Start of Directory
    if(checkSequence(START_OF_DIRECTORY, depth) != 0)
        return -1;
    int endDir = 0;
    while(endDir == 0){
        int c = getchar();
        if(c != MAGIC0 || c == EOF)
            return -1;
        c = getchar();
        if(c != MAGIC1 || c == EOF)
            return -1;
        c = getchar();
        if(c != MAGIC2 || c == EOF)
            return -1;
        c = getchar();
        if(c == END_OF_DIRECTORY){
            endDir = -1;
            break;
        }  
        if(c != DIRECTORY_ENTRY)
            return -1;
        if(BigEndianToInt(4) != depth)
            return -1;
        int size = BigEndianToInt(8);
        if(size < 29)
            return -1;
        mode_t mode = BigEndianToInt(4);
        entrySize = BigEndianToInt(8);
        size = size - 28;
        int i = 0;
        while(i < size){
            c = getchar();
            if(c == EOF)
                return -1;
            *(name_buf + i) = c;          
            i++;       
        }  
        *(name_buf + i) = '\0';
        if(S_ISREG(mode)){
            path_push(name_buf);
            deserialize_file(depth);
            if(global_options >> 3 == 1)
                chmod(path_buf, mode & 0777);
            path_pop();
        }
        if(S_ISDIR(mode)){
            path_push(name_buf);
            struct stat st;
            if(stat(path_buf, &st) == -1){
                mkdir(path_buf, 0700);
                chmod(path_buf, mode & 0777);
            }
            deserialize_directory(depth + 1);
            path_pop();
        }
    }
    if(BigEndianToInt(4) != depth)
        return -1;
    if(BigEndianToInt(8) != 16)
        return -1;
    return 0;
}

/*
 * @brief Deserialize the contents of a single file.
 * @details  This function assumes that path_buf contains the name of a file
 * to be deserialized.  The file must not already exist, unless the ``clobber''
 * bit is set in the global_options variable.  It reads (from the standard input)
 * a single FILE_DATA record containing the file content and it recreates the file
 * from the content.
 *
 * @param depth  The value of the depth field that is expected to be found in
 * the FILE_DATA record.
 * @return 0 in case of success, -1 in case of an error.  A variety of errors
 * can occur, including a depth field in the FILE_DATA record that does not match
 * the expected value, the record read is not a FILE_DATA record, the file to
 * be created already exists, or an I/O error occurs either while reading
 * the FILE_DATA record from the standard input or while re-creating the
 * deserialized file.
 */
int deserialize_file(int depth){
    if((global_options>>3) == 0){
        struct stat exists;
        if(stat(path_buf, &exists) == 0){
            return -1;
        }
    }
    int c = getchar();
    if(c != MAGIC0 || c == EOF)
        return -1;
    c = getchar();
    if(c != MAGIC1 || c == EOF)
        return -1;
    c = getchar();
    if(c != MAGIC2 || c == EOF)
        return -1;
    c = getchar();
    if(c != FILE_DATA || c == EOF)
        return -1;
    if(BigEndianToInt(4) != depth)
        return -1;
    int size = BigEndianToInt(8) - 16;
    if(size != entrySize){
        return -1;
    }
    int i = 0;
    FILE *f = fopen(path_buf, "w");
    while(i < size){
        c = getchar();
        if(c == EOF)
            return -1;
        fputc(c, f);
        i++;
    }
    fclose(f);
    return 0;
}

/*
 * @brief  Serialize the contents of a directory as a sequence of records written
 * to the standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * directory to be serialized.  It serializes the contents of that directory as a
 * sequence of records that begins with a START_OF_DIRECTORY record, ends with an
 * END_OF_DIRECTORY record, and with the intervening records all of type DIRECTORY_ENTRY.
 *
 * @param depth  The value of the depth field that is expected to occur in the
 * START_OF_DIRECTORY, DIRECTORY_ENTRY, and END_OF_DIRECTORY records processed.
 * Note that this depth pertains only to the "top-level" records in the sequence:
 * DIRECTORY_ENTRY records may be recursively followed by similar sequence of
 * records describing sub-directories at a greater depth.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open files, failure to traverse directories, and I/O errors
 * that occur while reading file content and writing to standard output.
 */
int serialize_directory(int depth) {
    // To be implemented.
    //steps:
    //start directory
    header(START_OF_DIRECTORY);
    int d = depth;
    intToBigEndianHex(d, 4);
    int a = 16;
    intToBigEndianHex(a, 8);
    //directories or files
    DIR *dir = opendir(path_buf);
    struct dirent *de;
    struct stat stat_buf;
    while((de = readdir(dir)) != NULL){
        char *name = de->d_name;
        if(cmp(name, ".") == 0 || cmp(name, "..") == 0)
            continue;
        header(DIRECTORY_ENTRY);
        intToBigEndianHex(d, 4);
        int size = 16 + 12 + strLen(name);
        intToBigEndianHex(size, 8);
        path_push(name);
        if(stat(path_buf, &stat_buf) == 0){
            size = stat_buf.st_mode;
            intToBigEndianHex(size, 4);
            size = stat_buf.st_size;
            intToBigEndianHex(size, 8);
        }
        charStr(name);
        if(S_ISREG(stat_buf.st_mode)){
            serialize_file(depth, stat_buf.st_size);     
        }
        if(S_ISDIR(stat_buf.st_mode)){
            serialize_directory(depth + 1);
        }
        path_pop();
    }
    closedir(dir);
    //end of directory
    header(END_OF_DIRECTORY);
    intToBigEndianHex(d, 4);
    intToBigEndianHex(a, 8);
    return 0;
}

/*
 * @brief  Serialize the contents of a file as a single record written to the
 * standard output.
 * @details  This function assumes that path_buf contains the name of an existing
 * file to be serialized.  It serializes the contents of that file as a single
 * FILE_DATA record emitted to the standard output.
 *
 * @param depth  The value to be used in the depth field of the FILE_DATA record.
 * @param size  The number of bytes of data in the file to be serialized.
 * @return 0 in case of success, -1 otherwise.  A variety of errors can occur,
 * including failure to open the file, too many or not enough data bytes read
 * from the file, and I/O errors reading the file data or writing to standard output.
 */
int serialize_file(int depth, off_t size) {
    // To be implemented.
    // file data
    int a = size + 16;
    header(FILE_DATA);
    num = 0;
    int d = (long int)depth;
    intToBigEndianHex(d, 4);
    num = 0;
    intToBigEndianHex(a, 8);    
    long int i = 0;
    FILE *f = fopen(path_buf, "r");
    while(i < size){
        unsigned char c = fgetc(f);
        putchar((int)c);
        if(i == size - 1)
            fclose(f);
        i++;
    }  
      
    return 0;  
}

/**
 * @brief Serializes a tree of files and directories, writes
 * serialized data to standard output.
 * @details This function assumes path_buf has been initialized with the pathname
 * of a directory whose contents are to be serialized.  It traverses the tree of
 * files and directories contained in this directory (not including the directory
 * itself) and it emits on the standard output a sequence of bytes from which the
 * tree can be reconstructed.  Options that modify the behavior are obtained from
 * the global_options variable.
 *
 * @return 0 if serialization completes without error, -1 if an error occurs.
 */
int serialize() {
    // To be implemented.
    header(START_OF_TRANSMISSION);
    intToBigEndianHex(0, 4);
    intToBigEndianHex(16, 8);
    serialize_directory(1);
    header(END_OF_TRANSMISSION);
    intToBigEndianHex(0, 4);
    intToBigEndianHex(16, 8);
    fflush(stdout);
    return 0;
}

/**
 * @brief Reads serialized data from the standard input and reconstructs from it
 * a tree of files and directories.
 * @details  This function assumes path_buf has been initialized with the pathname
 * of a directory into which a tree of files and directories is to be placed.
 * If the directory does not already exist, it is created.  The function then reads
 * from from the standard input a sequence of bytes that represent a serialized tree
 * of files and directories in the format written by serialize() and it reconstructs
 * the tree within the specified directory.  Options that modify the behavior are
 * obtained from the global_options variable.
 *
 * @return 0 if deserialization completes without error, -1 if an error occurs.
 */
int deserialize() {
    //Find the directory, if not present then create it
    //from the file read and make sure that it starts with START_TRANSMISSION 
    if(checkSequence(START_OF_TRANSMISSION, 0) != 0)
        return -1;
    struct stat st;
    if(stat(path_buf, &st) == -1)
        mkdir(path_buf, 0700);
    //call deserialize_directory
    deserialize_directory(1);
    //End of transmission
    if(checkSequence(END_OF_TRANSMISSION, 0) != 0)
        return -1;
    fflush(stdout);
    return 0;
}

/**
 * @brief Validates command line arguments passed to the program.
 * @details This function will validate all the arguments passed to the
 * program, returning 0 if validation succeeds and -1 if validation fails.
 * Upon successful return, the selected program options will be set in the
 * global variable "global_options", where they will be accessible
 * elsewhere in the program.
 *
 * @param argc The number of arguments passed to the program from the CLI.
 * @param argv The argument strings passed to the program from the CLI.
 * @return 0 if validation succeeds and -1 if validation fails.
 * Refer to the homework document for the effects of this function on
 * global variables.
 * @modifies global variable "global_options" to contain a bitmap representing
 * the selected options.
 */
int validargs(int argc, char **argv)
{
    char *empty = ".";
    if(argc < 2)
        return -1;
    char *strPtr = *(argv+1);
    if(cmp(strPtr, "-h") == 0){
        global_options = 1;
        return 0;
    }
    else if(cmp(strPtr, "-s") == 0){
        strPtr = *(argv+2);
        if(argc != 4 && argc != 2)
            return -1;
        if(argc == 2){
            path_init(empty);
        }
        if(argc == 4){
            if(cmp(strPtr, "-p") != 0)
                return -1;
            strPtr = *(argv+3);
            if(*(strPtr) == '-')
                return -1;
            path_init(strPtr);
        }
        global_options = 1<<1;  
        return 0;
    }
    else if(cmp(strPtr, "-d") == 0){
        if(argc > 5)
            return -1;
        if(argc == 3){
            strPtr = *(argv+2);
            if(cmp(strPtr, "-c") != 0)
                return -1;
            global_options = (1<<2)|(1<<3);
            path_init(empty);
        }
        if(argc == 4){
            strPtr = *(argv+2);
            if(cmp(strPtr, "-p") != 0)
                return -1;
            strPtr = *(argv+3);
            if(cmp(strPtr, "-c") == 0)
                return -1;
            path_init(strPtr);
            global_options = 1<<2;
        }
        if(argc == 5){
            strPtr = *(argv+2);
            if(cmp(strPtr, "-c") == 0){
                strPtr = *(argv+3);
                if(cmp(strPtr, "-p") != 0)
                    return -1;
                strPtr = *(argv+4);
                if(cmp(strPtr, "-c") == 0)
                    return -1;
                path_init(strPtr);
                global_options = (1<<2)|(1<<3);
            }
            else if(cmp(strPtr, "-p") == 0){
                strPtr = *(argv+3);
                if(cmp(strPtr, "-c") == 0)
                    return -1;
                path_init(strPtr);
                strPtr = *(argv+4);
                if(cmp(strPtr, "-c") != 0)
                    return -1;
                global_options = (1<<2)|(1<<3);
            }
            else
                return -1;
        }
        return 0;
    } 
    else
        return -1;
}

/*
 *Compares two strings.
 *Returns 0 if equal, -1 if not.
*/
int cmp(char *one, char *two){
    while(*two != '\0'){
        if(*one != *two)
            return -1;
        one++;
        two++;
    }
    if(*one != *two)
        return -1;
    return 0;    
}

/*
 * Turns the provided int into a big endian int for the output.
 * Used for depth and size in the sequence
*/
int intToBigEndianHex(int val, int byteSize){
    if(byteSize > 4){
        for(int i = 0; i < byteSize - 4; i++)
            putchar(0);
    }
    putchar((val>>24) & 0xff);
    putchar((val>>16) & 0xff);
    putchar((val>>8) & 0xff);
    putchar((val) & 0xff);
    return 0;
}

/*
 * Returns length of the string
*/
int strLen(char *str){
    int length = 0;
    while(*str != '\0'){
        length++;
        str++;
    }
    return length;
}

/*
 * Puts each character in the string in the stdout.
*/
int charStr(char *str){
    while(*str != '\0'){
        char c = *str;
        putchar((int)c);
        str++;
    }
    return 0;
}

/*
 *First five 4 bytes of each sequence
*/
int header(int type){
    putchar(MAGIC0);
    putchar(MAGIC1);
    putchar(MAGIC2);
    putchar(type);
    return 0;
}

/*
 * Method to check for 16 bytescheck for magic seq, take type,depth as an input
 * Only for records that are only just the header.
*/
int checkSequence(int type, int depth){
    int c = getchar();
    if(c != MAGIC0 || c == EOF){
        return -1;
    }
    c = getchar();
    if(c != MAGIC1 || c == EOF){
        return -1;
    }
    c = getchar();
    if(c != MAGIC2 || c == EOF){
        return -1;
    }
    c = getchar();
    if(c != type || c == EOF){
        return -1;
    }
    if(BigEndianToInt(4) != depth){
        return -1;
    }
    if(BigEndianToInt(8) != 16){
        return -1;
    }
    return 0;
}

//Read big endian values and return an int.
int BigEndianToInt(int numBytes){
    int res = 0;
    int count = 0;
    for(int i = 0; i < numBytes; i++){
        int c = getchar();
        if(c == EOF)
            return -1;
        if(c == 0 && res == 0)
            continue;
        if(res == 0){
            res += c;
            count++;
        }
        else    
            res = res<<8 | c;
    }
    return res;
}


