#include <Windows.h>
#include <iostream>
#include "SP3DLL.h"

int main()
{
    // Вызов функции из DLL
    const char* targetString = "Hello";
    const char* replacementString = "World";
    FindAndReplaceString(targetString, replacementString);

    return 0;
}