import subprocess
import time

_ID_FORMAT = '%s %s %s'

_ID_WRAPPER = '''
#include "VERSION.h"

#if C4DEBUG
char _BuildId[] = "D " "%(build_id)s";
#else
char _BuildId[] = "R " "%(build_id)s";
#endif
'''

def GET_BUILD_ID():
    p = subprocess.Popen(['git', 'log', '-1', '--format=%H'], shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    lines = p.stdout.readlines()
    retval = p.wait()

    commit_hash = lines[0][0:-1]
    
    p = subprocess.Popen(['git', 'rev-parse', '--abbrev-ref', 'HEAD'], shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    lines = p.stdout.readlines()
    retval = p.wait()
    branch_name = lines[0][0:-1]
    
    p = subprocess.Popen(['git', 'status', '-s'], shell=False, stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
    lines = p.stdout.readlines()
    retval = p.wait()
    
    if len(lines) > 0:
        commit_hash = '!' + commit_hash
    
    return _ID_FORMAT % (time.strftime("%a %x @ %X"), branch_name, commit_hash)
   

def WRAPPED_BUILD_ID():
    return _ID_WRAPPER % {'build_id' : GET_BUILD_ID()}

if __name__ == '__main__':
    print WRAPPED_BUILD_ID()
