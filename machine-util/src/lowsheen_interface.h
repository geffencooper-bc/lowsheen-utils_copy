

#include <string>
#include "manifest.h"
#include "lowsheen_headers.h"

#pragma once

namespace lowsheen
{
    class MuleInterface
    {
        public:
            bool enter_normal_mode();
            bool get_header(header_t *header);
            bool enter_xt_can_mode();
    };
}