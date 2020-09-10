

#include <string>
#include "manifest.h"
#include "lowsheen_headers.h"

namespace lowsheen
{
    class MuleInterface
    {
        private:
            Manifest manifest;
        public:
            MuleInterface(const char *filename);
            bool enter_normal_mode();
            header_t get_header();
            bool enter_xt_can_mode();
    };
}