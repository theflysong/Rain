#include "file/posinfo.h"

using namespace rain;

mem::Pool<PosInfo> PosInfo::pool = mem::Pool<PosInfo>(1000);