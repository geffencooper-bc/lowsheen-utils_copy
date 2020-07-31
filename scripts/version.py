#!//usr/bin/env python
# ============================================================================
# Copyright 2020 Brain Corporation. All rights reserved. Brain
# Corporation proprietary and confidential.
# ALL ACCESS AND USAGE OF THIS SOURCE CODE IS STRICTLY PROHIBITED
# WITHOUT EXPRESS WRITTEN APPROVAL FROM BRAIN CORPORATION.
# Portions of this Source Code and its related modules/libraries
# may be governed by one or more third party licenses, additional
# information of which can be found at:
# https://info.braincorp.com/open-source-attributions
# ============================================================================
from subprocess import Popen, PIPE
from os.path import isfile
from datetime import datetime

if __name__ == "__main__":
    
    filename = "version.h"

    p1 = Popen(('git', 'rev-parse', 'HEAD'), stdout=PIPE, stderr=PIPE)
    p2 = Popen(('git', 'describe'), stdout=PIPE, stderr=PIPE)
   
    git_hash = p1.communicate()[0].strip()
    git_tag = p2.communicate()[0].strip()
    date_local = str(datetime.now().strftime("%Y-%m-%d"))
 
    if git_hash == "":
        git_hash = "No GIT Hash"

    if git_tag == "":
        git_tag = "No GIT Tag"

    version_string = ""
    version_string += "#define VERSION_GIT_HASH \"" + git_hash + "\"\n"
    version_string += "#define VERSION_GIT_TAG \"" + git_tag + "\"\n"
    version_string += "#define VERSION_GIT_DATE_LOCAL \"" + date_local + "\"\n"
 
    # verify if there is already a generate version file, and match contents
    if isfile(filename):
        with open(filename, 'r') as version_file:
            version_data = version_file.read()  
            if version_data == version_string:
                exit(0) # no need to overwrite file with same content

    with open(filename, 'w') as version_file:
        version_file.write(version_string)

        

