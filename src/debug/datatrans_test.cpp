#include<iostream>
#include<datatrans.h>


int main()
{
        std::string src,des;
        std::cin>>src>>des;

        cp_file(src,des);

        return 0;
}