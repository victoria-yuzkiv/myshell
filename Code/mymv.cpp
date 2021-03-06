#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <cstring>
#include <unistd.h>
#include <map>

#define BOOST_NO_CXX11_SCOPED_ENUMS
#include <boost/filesystem.hpp>
#undef BOOST_NO_CXX11_SCOPED_ENUMS

using namespace boost::filesystem;
using namespace std;

//Functions for displaying an output
int writebuffer (int fd, const char* buffer , ssize_t size , int* status ){
    ssize_t written_bytes = 0;
    while( written_bytes < size ) {
        ssize_t written_now = write(fd, buffer + written_bytes, size - written_bytes );
        if( written_now == -1){
            if (errno == EINTR)
                continue;
            else{
                *status = errno;
                return -1;
            }
        }else
            written_bytes += written_now;
    }
    return 0;
}

void print_sth(string info){
    int status;
    int out;
    out = writebuffer(STDOUT_FILENO, info.c_str(), info.length(), &status);
    if (out){
        cerr << "Writebuffer error" << endl;
    }
}

int dir_copying( boost::filesystem::path const & source,
                 boost::filesystem::path const & destination )
{
    try
    {
        // Check whether the function call is valid
        if( !boost::filesystem::exists(source) ||
            !boost::filesystem::is_directory(source) )
        {
            std::cerr << "Source directory "
                      << source.string()
                      << " does not exist or is not a directory."
                      << '\n';
            return 1;
        }

        if( boost::filesystem::exists( destination ) )
        {
            std::cerr << "Destination directory "
                      << destination.string()
                      << " already exists." << '\n';
            return 1;
        }

        // Create the destination directory
        if( !boost::filesystem::create_directory( destination ) )
        {
            std::cerr << "Unable to create destination directory"
                      << destination.string() << '\n';
            return 1;
        }
    }

    catch( boost::filesystem::filesystem_error const & e)
    {
        std::cerr << e.what() << '\n';
        return 1;
    }

    // Iterate through the source directory
    for( boost::filesystem::directory_iterator file( source );
         file != boost::filesystem::directory_iterator();
         ++file )
    {
        try
        {
            boost::filesystem::path current( file->path() );
            if( boost::filesystem::is_directory( current ) )
            {
                // Found directory: Recursion
                if( !dir_copying( current, destination / current.filename() ) )
                {
                    return 1;
                }
            }
            else
            {
                // Found file: Copy
                rename( current, destination / current.filename() );
            }
        }

        catch( boost::filesystem::filesystem_error const & e )
        {
            std:: cerr << e.what() << '\n';
        }
    }
    return 0;
}

void file_copying(path from, path to, map<string,int> &commands){
    if(commands["-f"]){
        rename(from, to);
    }else {
        if(exists(to)){
            print_sth(
                    "Enter Y if you want to overwrite the existing file and N otherwise. Also, press A to overwrite all next files or C to cancel this operation): \n");
            string choise;
            cin >> choise;
            transform(choise.begin(), choise.end(), choise.begin(), ::tolower);
            if (choise == "y") {
                rename(from, to);
            } else if (choise == "n") {
            } else if (choise == "c") {
            } else if(choise == "a"){
                commands["-f"] = 1;
            } else {
                print_sth("No such choise!\n");
            }
        }else{
            rename(from, to);
        }
    }
}

int main(int argc, char** argv) {
    int status;
    int all = 0;
    vector<string> files;
    if (argc < 2) {
        cerr << "You didn't enter any info" << endl;
        return 1;
    }else if(argc < 3){
        cerr << "Enter more than 1 name" << endl;
        return 1;
    }
    // Map of commands
    map<string, int> commands = {{"-f", 0}};
    int outinfo;
    //  ifstream file;
    int check = 0;
    for (int i = 1; i < argc; ++i) {
        string tmp(argv[i]);
        if (tmp == "-h" || tmp == "--help") {
            string desc = "Use mymv to move files or directories. Use -f to overwrite all files";
            print_sth(desc);
        } else if (!(commands.find(tmp) == commands.end()))
            commands[tmp] = 1;
        else {
            files.push_back(tmp);
            check = 1;
        }
    }
    if(check){
        string last;
        if (!files.empty()) {
            last = files.back();
            files.pop_back();
        } else {
            print_sth("You didn't enter any info!\n");
            return 1;
        }
        for (string object : files) {
            if (is_directory(path(last))) {
                for (string obj : files) {
                    if (exists(path(obj))) {
                        if (!(commands["-R"]) && is_directory(path(obj))) {
                            print_sth("Use -R to copy directories!\n");
                            return 1;
                        }
                    } else {
                        print_sth("Bad Parameters!\n");
                        return 1;
                    }
                }
                if (is_directory(path(object))) {
                    dir_copying(path(object), path(last) / path(object).filename());
                } else if (is_regular_file(path(object))) {
                    file_copying(path(object), path(last) / path(object), ref(commands));
                }
            }else if (is_regular_file(path(files.front()))) {
                if (files.size() != 1) {
                    print_sth("Bad parameters!\n");
                    return 1;
                }
                file_copying(path(files.front()), path(last), ref(commands));
            }else{
                print_sth("Bad Parameters!\n");
                return 1;
            }
        }
    }
    return 0;
}