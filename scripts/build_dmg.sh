current=$(pwd)
cd ../bin/Release
name=ImageEn_DecryptoUI
mkdir $name
cp -R $name.app $name/
# size=$(du -sh ./$name.app | awk '{print $1}')
size=500M
hdiutil create -size $size -fs HFS+ -volname "$name" -o $name.dmg
hdiutil attach $name.dmg
cp -R $name/* /Volumes/$name/
cd "$current"
