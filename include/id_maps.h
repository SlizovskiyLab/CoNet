#ifndef ID_MAPS_H
#define ID_MAPS_H

#include <unordered_map>
#include <string>

extern const std::unordered_map<int, std::string> argIdMap;
extern const std::unordered_map<int, std::string> mgeIdMap;
extern const std::unordered_map<int, std::string> argGroupMap;

std::string getARGName(int id);
std::string getMGEName(int id);
std::string getARGGroupName(int id);

int getARGId(const std::string& name);
int getMGEId(const std::string& name);
int getARGGroupId(const std::string& name);

#endif // ID_MAPS_H
