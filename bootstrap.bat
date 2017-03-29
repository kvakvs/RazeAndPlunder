rem RUN this script to check out development BWAPI into deps/bwapi directory

git clone https://github.com/kvakvs/bwapi.git deps/bwapi
cd deps/bwapi
git remote add bwapi https://github.com/bwapi/bwapi.git
git remote add origin-write git@github.com:kvakvs/bwapi.git
git checkout develop
