#ifndef EXTERNAL_COMPONENTS_STORAGE_H
#define EXTERNAL_COMPONENTS_STORAGE_H

#include <string>
#include <vector>
#include <map>

class Storage {
public:
    Storage(const std::string& platform, const std::vector<std::map<std::string, std::string>>& files);
    void initialize();
    
private:
    std::string platform;
    std::vector<std::map<std::string, std::string>> files;
    
    void setup_flash();
    void setup_inline();
};

#endif
