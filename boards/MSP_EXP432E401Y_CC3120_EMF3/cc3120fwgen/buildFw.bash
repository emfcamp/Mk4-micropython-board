#!/bin/bash
SLICPATH=$1/simplelink/imagecreator/bin
SDKINSTALLPATH=$2

BASEPATH=`pwd`
BASEFSPATH=$BASEPATH/fs
UNIFLASHICPROJPATH=`pwd`/projects

RUNCMD=./SLImageCreator
PROJNAME=cc3120fw
DEVNAME=CC3120R
DEVMODE=production

install -d `pwd`/projects

pushd $SLICPATH

echo Create New Project
$RUNCMD project new --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --device $DEVNAME --mode $DEVMODE --overwrite

echo Setting Service Pack
$RUNCMD project set_sp --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --file "$SDKINSTALLPATH/tools/cc31xx_tools/servicepack-cc3x20/sp_3.7.0.1_2.0.0.0_2.2.0.6.bin"

echo Setting Certificate Store
$RUNCMD project set_certstore --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --file "$SDKINSTALLPATH/tools/cc31xx_tools/certificate-catalog/certcatalog20171221.lst" --sign "$SDKINSTALLPATH/tools/cc31xx_tools/certificate-catalog/certcatalog20171221.lst.signed.bin"

echo Adding WPA Enterprise Root CA Chain [/sys/cert/ca.der]
$RUNCMD project add_file --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --fs_path "/sys/cert/ca.der" --file "$BASEFSPATH/radius.emfcamp.org.chain.pem" --max_size 8192

echo Adding Cert [DST Root CA X3]
$RUNCMD project add_file --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --fs_path "DST Root CA X3" --file "$BASEFSPATH/DST_Root_CA_X3.der"

echo Adding Cert [Let s Encrypt Authority X3]
$RUNCMD project add_file --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --fs_path "Let's Encrypt Authority X3" --file "$BASEFSPATH/Let_s_Encrypt_Authority_X3.der"

echo Adding Cert [iot.eclipse.org]
$RUNCMD project add_file --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --fs_path "iot.eclipse.org" --file "$BASEFSPATH/iot.eclipse.org.der"

echo Adding Cert [badgeserver.emfcamp.org NOTE :: only valid till 19Nov2018]
$RUNCMD project add_file --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --fs_path "badgeserver.emfcamp.org" --file "$BASEFSPATH/badgeserver.emfcamp.org.der"

sed -i s/\"START_ROLE\":\"[0-9]\"/\"START_ROLE\":\"0\"/ $BASEPATH/projects/$PROJNAME/$PROJNAME.json

echo Generating an image file that can be used later
$RUNCMD project create_image --project_path "$UNIFLASHICPROJPATH" --name $PROJNAME --file "$BASEPATH/$PROJNAME.sli"

cp "$UNIFLASHICPROJPATH/$PROJNAME/sl_image/Output/Programming.ucf" "$BASEPATH/$PROJNAME.ucf"

popd
