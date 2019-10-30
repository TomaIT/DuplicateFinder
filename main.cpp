/*
 * To Compile g++ -O2 -std=c++14 main.cpp -lcrypto
 * Command to delete files: cat duplicates.txt | xargs rm | sh
 */

#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <unordered_map>
#include <string>
#include <iostream>
#include <vector>
#include <algorithm>
#include <stdlib.h>
#include <chrono>

#define BUFFER_SIZE 1024*1024

using namespace std;
unordered_map < long , vector<string> > mapSameSize;

string getDigest(const unsigned char* digest,size_t len){
    static const char hexchars[] = "0123456789abcdef";

    string result;

    for (int i = 0; i < len; i++)
    {
        unsigned char b = digest[i];
        char hex[3];

        hex[0] = hexchars[b >> 4];
        hex[1] = hexchars[b & 0xF];
        hex[2] = 0;

        result.append(hex);
    }
    return result;
}

string md5(const char *filename){
    unsigned char c[MD5_DIGEST_LENGTH];
    int i;
    FILE *inFile = fopen (filename, "rb");
    MD5_CTX mdContext;
    int bytes;
    unsigned char data[BUFFER_SIZE];
    if (inFile == nullptr) {
        printf ("%s can't be opened.\n", filename);
        return nullptr;
    }
    MD5_Init (&mdContext);
    while ((bytes = fread (data, 1, BUFFER_SIZE, inFile)) != 0)MD5_Update (&mdContext, data, bytes);
    MD5_Final (c,&mdContext);
    fclose (inFile);
    return getDigest(c,MD5_DIGEST_LENGTH);
}

string sha256(const char *filename){
    unsigned char c[SHA256_DIGEST_LENGTH];
    int i;
    FILE *inFile = fopen (filename, "rb");
    SHA256_CTX sha256;
    int bytes;
    unsigned char data[BUFFER_SIZE];
    if (inFile == nullptr) {
        printf ("%s can't be opened.\n", filename);
        return nullptr;
    }
    SHA256_Init(&sha256);
    while ((bytes = fread (data, 1, BUFFER_SIZE, inFile)) != 0)SHA256_Update(&sha256, data, bytes);
    SHA256_Final(c, &sha256);
    fclose (inFile);
    return getDigest(c,SHA256_DIGEST_LENGTH);
}

void createMapSameSize(const char *name){
    DIR *dir;
    struct dirent *entry;
    if (!(dir = opendir(name)))return;

    while ((entry = readdir(dir)) != nullptr) {
        char path[1024];
        snprintf(path, sizeof(path), "%s/%s", name, entry->d_name);
        if (entry->d_type == DT_DIR) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)continue;
            createMapSameSize(path);
        } else {
            struct stat st{};
            stat(path, &st);
            long x=st.st_size;
            if(mapSameSize.find(x)==mapSameSize.end()){
                vector<string> s;
                s.emplace_back(path);
                mapSameSize[x]=s;
            }else{
                mapSameSize[x].push_back(path);
            }
        }
    }
    closedir(dir);
}

long epochTime(const char *fileName){
    struct stat st{};
    stat(fileName, &st);
    return st.st_ctim.tv_sec;
}

unordered_map <string,vector<string>> getSameMd5(const vector<string>& files){
    unordered_map <string,vector<string>> temp;

    for(const auto& i:files){
        string m=md5(i.c_str());
        if(temp.find(m)==temp.end()){
            vector<string> s;
            s.emplace_back(i);
            temp[m]=s;
        }else{
            temp[m].push_back(i);
        }
    }

    return temp;
}

unordered_map <string,vector<string>> getSameSha256(const vector<string>& files){
    unordered_map <string,vector<string>> temp;

    for(const auto& i:files){
        string m=sha256(i.c_str());
        if(temp.find(m)==temp.end()){
            vector<string> s;
            s.emplace_back(i);
            temp[m]=s;
        }else{
            temp[m].push_back(i);
        }
    }

    return temp;
}

void writeDuplicated(FILE *fout,vector<string> fileNames){
    long min=epochTime(fileNames[0].c_str());
    int index=0;
    for(int j=1;j<fileNames.size();j++){
        long temp=epochTime(fileNames[j].c_str());
        if(temp<min){
            min=temp;
            index=j;
        }
    }
    for(int j=0;j<fileNames.size();j++){
        if(j==index)continue;
        fprintf(fout,"'%s'\n",fileNames[j].c_str());
    }
    fflush(fout);
}

void findDuplicated(){
    FILE *fout=fopen("duplicates.txt","w");
    for(const auto& elem : mapSameSize){
        if(elem.second.size()<=1)continue;

        //HERE SAME SIZE

        for(const auto& i:getSameMd5(elem.second)){
            if(i.second.size()<=1)continue;

            //HERE SAME SIZE AND SAME MD5

            writeDuplicated(fout,i.second);


            cout<<elem.first<<" - "<<i.first<<endl;
            for(const auto& j:i.second){
                cout<<j<<endl;
            }
            cout<<endl;
            cout<<endl;

            /*for(const auto& a:getSameSha256(i.second)){
                if(a.second.size()<=1)continue;

                //HERE SAME SIZE, SAME MD5 AND SAME SHA256

            }*/
        }
    }
    fclose(fout);
}

int main(int argc,char **argv) {
    if(argc<=1){
        printf("Error paramaters. Example:\n%s rootPath_1/ rootPath_2/ ...",argv[0]);
        return 1;
    }
    for(int i=1;i<argc;i++)createMapSameSize(argv[i]);
    findDuplicated();
    return 0;
}