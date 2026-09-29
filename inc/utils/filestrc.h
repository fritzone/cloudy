#ifndef FILESTRC_H
#define FILESTRC_H
/**
 * Holds the data of a file, and is used to render it
 */
struct FileStructure
{
    // the name of the file, for remote files this is the full (long) name
    char* sname;

    // for remote files the hash the peer uses to identify it, NULL for local files
    char* hash;

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
