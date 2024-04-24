#include <iostream>
#include <string>
#include <list>
#include <fstream>
#include <vector>

struct data {
    int pid = -1;
    bool isHole = false;
    int length = -1;
};

class MemoryManager {
public:
    int policy = 0;
    std::list<data> space;
    MemoryManager(int bytes, int policy) {
        data Hole;
        Hole.length = bytes;
        Hole.isHole = true;
        space.push_back(Hole);
        std::cout << "Memory started with " << bytes << " bytes and policy: " << policy << "!" << std::endl;
    }
    void mergeNeighboringHoles() {
        for (int i = 0; i < space.size(); i++) {
            // make sure we run thru the array in pairs
            std::list<data>::iterator temp = space.begin();
            std::list<data>::iterator prev = space.begin();
            if (temp != space.end()) {
                temp++;
            }
            else {
                // if they are the only two and both are holes, merge
                if (temp->isHole && prev->isHole) {
                    int newlength = temp->length + prev->length;
                    prev->length = newlength;
                    space.erase(temp);
                }
                return;
            }
            while (temp != space.end()) {
                if (temp != space.end()) {
                    if (temp->isHole && prev->isHole && prev != temp) {
                        // merge
                        data newHole;
                        newHole.length = prev->length + temp->length;
                        newHole.isHole = true;

                        int holes = 0;
                        for (auto n : space) {
                            if (n.isHole) {
                                holes += 1;
                            }
                        }
                        std::cout << "There are " << holes << " holes currently." << std::endl;

                        space.insert(prev, newHole);
                        space.erase(prev);
                        space.erase(temp);
                        std::cout << "Removed hole via merge!" << std::endl;

                        holes = 0;
                        for (auto n : space) {
                            if (n.isHole) {
                                holes += 1;
                            }
                        }
                        std::cout << "There are " << holes << " holes currently." << std::endl;

                        break;
                    }
                    temp++;
                    prev++;
                }
                else {
                    break;
                }
            }
        }
    }
    int allocate(int bytes, int pid) {
        bool found = false;
        // find hole with big enough space (first fit)
        if (policy == 0) {
            std::list<data>::iterator it = space.begin();
            while (it != space.end()) {
                if (it->isHole && it->length >= bytes) {
                    // good
                    found = true;
                    int canuse = it->length;

                    data newProcess;
                    newProcess.pid = pid;
                    newProcess.isHole = false;
                    newProcess.length = bytes;

                    data newHole;
                    newHole.length = canuse - bytes;
                    newHole.isHole = true;

                    std::cout << "New process is " << newProcess.length << " long" << std::endl;
                    std::cout << "New hole is " << newHole.length << " long" << std::endl;

                    space.erase(it);

                    space.push_back(newProcess);
                    space.push_back(newHole);

                    break;
                }
                it++;
            }
        }
        // find hole with small enough space but big enough (best fit)
        else if (policy == 1) {
            std::list<data>::iterator it = space.begin();
            std::vector<std::list<data>::iterator> possiblefits;
            while (it != space.end()) {
                if (it->isHole && it->length == bytes) {
                    // first best fit, use this
                    found = true;
                    it->pid = pid;
                    it->isHole = false;
                    std::cout << "found PERFECT fit hole and turned it into process w/ length " << it->length << " bytes!" << std::endl;
                    break;
                }
                else if (it->isHole && it->length > bytes) {
                    possiblefits.push_back(it);
                }
                it++;
            }
            if (!found && possiblefits.size() > 0) {
                found = true;
                std::list<data>::iterator* bestfit = &possiblefits[0];
                // loop thru possible holes and find one with lowest length
                for (int i = 0; i < possiblefits.size(); i++) {
                    if (possiblefits[i]->length < (*bestfit)->length) {
                        bestfit = &possiblefits[i];
                    }
                }
                // break the hole up so it becomes a process followed by hole
                data newProcess;
                newProcess.pid = pid;
                newProcess.isHole = false;
                newProcess.length = bytes;

                data newHole;
                newHole.length = (*bestfit)->length - bytes;
                newHole.isHole = true;

                std::cout << "New process is " << newProcess.length << " long" << std::endl;
                std::cout << "New hole is " << newHole.length << " long" << std::endl;

                space.insert(*bestfit, newHole);
                space.insert(*bestfit, newProcess);

                space.erase(*bestfit);

            }
            else if (!found && possiblefits.size() == 0) {
                // see if theres enough space to do compaction and get some memory free
                int freeSpace = 0;
                for (auto n : space) {
                    if (n.isHole) {
                        freeSpace += n.length;
                    }
                }
                if (freeSpace >= bytes) {
                    std::cout << "There is enough free space but compaction must occur first." << std::endl;
                    defragment();
                    allocate(bytes, pid);
                    return 0;
                }
            }
        }

        if (!found) {
            std::cout << "Could not find suitable hole to allocate data to." << std::endl;
            return -1;
        }
        mergeNeighboringHoles();
        std::cout << "successfull allocation of " << bytes << " bytes!" << std::endl;
        return 1;
    }
    int deallocate(int pid) {
        // find the process based on pid
        std::cout << "de-allocating process " << pid << std::endl;
        std::list<data>::iterator it = space.begin();
        while (it != space.end()) {
            if (it->pid == pid && it->isHole == false) {
                //  just turn it into a hole
                it->isHole = true;
                it->pid = -1;
                mergeNeighboringHoles();
                return 1;
            }
            it++;
        }
        std::cout << "Failed to de-allocate process with id: " << pid << ", does not seem to exist!" << std::endl;
        return -1;
    }
    void print() {

        int freeSpace = 0;
        int usedSpace = 0;
        int totalSpace = 0;
        int holes = 0;
        int processes = 0;
        int locationHolder = 0;
        std::string HoleList = "";
        std::string ProcessList = "";
        std::string vir = "";
        std::cout << std::endl;
        std::cout << "### PRINTING MEMORY ###" << std::endl;
        int holeTag = 1;
        for (data n : space) {
            if (!n.isHole) {
                usedSpace += n.length;
                for (int i = 0; i < n.length; i++) {
                    vir += std::to_string(n.pid);
                }
                processes++;
                ProcessList += "Process ID=" + std::to_string(n.pid) + ", start location=" + std::to_string(locationHolder) + ", size=" + std::to_string(n.length) + '\n';
            }
            else {
                for (int i = 0; i < n.length; i++) {
                    vir += "_";
                }
                holes++;
                freeSpace += n.length;
                HoleList += "Hole #" + std::to_string(holeTag++) + ", start location=" + std::to_string(locationHolder) + ", size=" + std::to_string(n.length) + '\n';
            }
            locationHolder += n.length;
        }
        totalSpace += freeSpace + usedSpace;
        std::cout << "Memory Size: " << totalSpace << " bytes." << std::endl;
        std::cout << "Total amount of free space: " << freeSpace << "bytes, used space: " << usedSpace << "bytes" << std::endl;
        std::cout << "# of Holes = " << holes << " | # of Processes = " << processes << std::endl;
        std::cout << "Listing Processes:" << std::endl << ProcessList << std::endl;
        std::cout << "Listing Holes: " << std::endl << HoleList << std::endl;
        std::cout << "### END ###" << std::endl << std::endl;
        std::cout << vir << std::endl << std::endl;
    }
    int defragment() {
        // find all holes and calculate new hole size
        // remove all holes
        // add them up
        // create one hole and put it at the back
        int newSize = 0;
        std::list<data>::iterator it = space.begin();
        while (it != space.end()) {
            if (it->isHole && it != space.end()) {
                newSize += it->length;
                space.erase(it);
                break;
            }
            it++;
        }
        data newHole;
        newHole.length = newSize;
        newHole.isHole = true;
        space.push_back(newHole);
        mergeNeighboringHoles();

        int holes = 0;
        for (auto n : space) {
            if (n.isHole) {
                holes += 1;
            }
        }
        if (holes >= 2) {
            defragment();
            return 0;
        }
        std::cout << "Defragmented!" << std::endl;
        return 1;
    }
};

std::vector<std::string> splitString(std::string s) {
    // seperate by space
    int stringcount = 0;
    std::string segment = "";
    std::vector<std::string> strings;
    for (int i = 0; i < s.length(); i++) {
        if (s.substr(i, 1) == " ") {
            stringcount++;
            strings.push_back(segment);
            segment = "";
        }
        else {
            segment += s.substr(i, 1);
        }
    }
    // so it can push back the last segment
    strings.push_back(segment);
    return strings;
}

int main() {
    MemoryManager* m = nullptr;

    std::ifstream file;
    file.open("instructions.txt");
    std::string reading;
    int linenumber = 1;
    if (!file.fail()) {
        std::cout << "Good file" << std::endl;
        for (std::string line; std::getline(file, line);) {
            std::vector<std::string> contents = splitString(line);
            if (linenumber == 1) {
                std::cout << contents[0] << " and " << contents[1] << std::endl;
                m = new MemoryManager(std::stoi(contents[0]), std::stoi(contents[1]));
            }
            else {
                if (contents[0] == "A") {
                    m->allocate(std::stoi(contents[1]), std::stoi(contents[2]));
                }
                else if (contents[0] == "D") {
                    m->deallocate(std::stoi(contents[1]));
                }
                else if (contents[0] == "P") {
                    m->print();
                }
            }
            linenumber++;
        }
    }
    else {
        std::cout << "problem opening file :(" << std::endl;
    }

    if (m != nullptr) {
        delete m;
    }
    
    file.close();
    
    return 0;
}