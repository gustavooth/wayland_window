echo "Compiling window..."

buildf="build"
if [ ! -d "$buildf" ]; then
    mkdir "$buildf"
fi

cd $buildf
cmake -G Ninja ..
ninja