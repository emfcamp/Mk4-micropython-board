import network

def attribsToText(attrib):
    attribText = ""
    if attrib & 0x001:
        attribText += "OpFileCmt "
    else:
        attribText += "          "
    if attrib & 0x002:
        attribText += "OpBndlCmt "
    else: 
        attribText += "          "
    if attrib & 0x004:
        attribText += "PendFileCmt "
    else: 
        attribText += "            "
    if attrib & 0x008:
        attribText += "PendBndlCmt "
    else: 
        attribText += "            "
    if attrib & 0x010:
        attribText += "Secure "
    else: 
        attribText += "       "
    if attrib & 0x020:
        attribText += "           "
    else: 
        attribText += "  FailSafe "
    if attrib & 0x040:
        attribText += "System "
    else: 
        attribText += "User   "
    if attrib & 0x080:
        attribText += "SysWUser "
    else: 
        attribText += "         "
    if attrib & 0x100:
        attribText += "NoValidCpy "
    else: 
        attribText += "           "
    if attrib & 0x200:
        attribText += "PubWr"
    else: 
        attribText += "     "
    if attrib & 0x400:
        attribText += "PubRd"
    else: 
        attribText += "     "
    return(attribText)

w = network.WLAN()
w.active(True)

infoList = w.fsinfo()

deviceUsage = infoList[0]
filesUsage = infoList[1]

print("\n\r*** Device Usage ***") 
print("DeviceBlockSize                   : ", str(deviceUsage[0]))
print("DeviceBlocksCapacity              : ", str(deviceUsage[1]))
print("NumOfAllocatedBlocks              ; ", str(deviceUsage[2]))
print("NumOfReservedBlocks               : ", str(deviceUsage[3]))
print("NumOfReservedBlocksForSystemfiles : ", str(deviceUsage[4]))
print("LargestAllocatedGapInBlocks       : ", str(deviceUsage[5]))
print("NumOfAvailableBlocksForUserFiles  : ", str(deviceUsage[6]))

print("\n\r*** Files Usage ***") 
print("MaxFsFiles                        : ", str(filesUsage[0]))
print("IsDevlopmentFormatType            : ", str(filesUsage[1]))
print("Bundlestate                       ; ", str(filesUsage[2]))
print("MaxFsFilesReservedForSysFiles     : ", str(filesUsage[3]))
print("ActualNumOfUserFiles              : ", str(filesUsage[4]))
print("ActualNumOfSysFiles               : ", str(filesUsage[5]))
print("NumOfAlerts                       : ", str(filesUsage[6]))
print("NumOfAlertsThreshold              : ", str(filesUsage[7]))
print("FATWriteCounter                   : ", str(filesUsage[8]))

print("\n\r*** Summary ***") 
print("Total    Space : TODO")
print("User     Size  : TODO")
print("User     Avail : TODO")
print("System   Size  : TODO")
print("System   Avail : TODO")
print("Reserved Size  : TODO")

filesList = w.fslist()

print("\n\r *** Directory List ***")

for file in filesList:
   print("%-30s %8d %8d 0x%08x [%s]" % (file[0], file[1], file[2], file[3], attribsToText(file[3])))


