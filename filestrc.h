#ifndef FILESTRC_H
#define FILESTRC_H
/**
 * Holds the data of a file, and is used to render it
 */
struct FileStructure
{
    char* sname;

    unsigned long file_size;

    unsigned short day;
    unsigned short month;
    unsigned short year;
    unsigned short hour;
    unsigned short minute;
    unsigned short sec;

    bool is_dir;

    bool is_selected;

};

#endif // FILESTRC_H
