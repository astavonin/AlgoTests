#include "support.hpp"

int main(int argc, char* argv[])
{
    if(argc < 2)
    {
        std::cout << "Usage:\n\tsort_checker file_name" << std::endl;
        return -1;
    }
    const std::string input(argv[1]);

    isSorted<uint32_t>(input);

    return 0;
}

