#include<stdio.h>
#include<stdlib.h>
#include<string.h>

int main(){
    char buf[8192];
    gets(buf);
    printf("%s\n", buf);
    char* index2, *index1, *value;
    index1 = buf;
    index2 = strstr(index1, "Content-Length: ");
    value = (char *) malloc((index2+strlen("Content-Length: ")-index1+1)*sizeof(char));
    memcpy(value, index2, strlen("Content-Length: "));
    value[strlen("Content-Length: ")] = '\0';
    printf("%s\n", value);
    return 0;
}