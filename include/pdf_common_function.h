/*
 * @Author: mikey.yuanjian 
 * @Date: 2019-06-15 15:34:29 
 * @Last Modified by:   mikey.yuanjian 
 * @Last Modified time: 2019-06-15 15:34:29 
 */
#ifndef _PDF_COMMON_FUNCTION_H
#define _PDF_COMMON_FUNCTION_H
#include <stdio.h>
#include <sstream>
#include <iostream>
#ifdef __cplusplus
extern "C" {
#endif
namespace pdf {
//get object number (example:3 0 R)
static int GetObjectNumber(const std::string& input) {
    std::stringstream sstream_obj(input);
    std::string obj_string;
    sstream_obj >> obj_string;
    return atoi(obj_string.c_str());
}
static int GetColorspaceBydevice(const std::string& input) {
    if (input == "DeviceGray")
        return 1;
    else if (input == "DeviceCMYK")
        return 4;
    else
        return 3;
}
static int HexToNum(char input) {
    char dCh = 0;
    if (input >= '0' && input <= '9') {
        dCh = input - '0';
    } else if (input >= 'A' && input <= 'F') {
        dCh = (input - 'A') + 10;
    } else if (input >= 'a' && input <= 'f') {
        dCh = (input - 'a') + 10;
    } else {
        std::cout << "incorrect hex char!" << std::endl;
        return 0;
    }
    return dCh;
}
}  //namespace pdf
#ifdef __cplusplus
}
#endif
#endif  // _PDF_COMMON_FUNCTION_H
