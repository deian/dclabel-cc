#!/bin/bash
git clone ssh://anonymous@gitstar.com/scs/lio.git
cabal-dev add-source lio/quickcheck-lio-instances
cabal-dev install directory QuickCheck lio quickcheck-instances quickcheck-lio-instances
cabal-dev install 
./dist/build/genTest/genTest
pushd ../
make
popd
for i in /tmp/gen_tests/*.cc; do
  echo -n Compiling $i...
  g++ -o $i.out $i ../dclabel.o -I..;
  echo DONE
  echo -n Running $i...
  $i.out
  echo DONE
done
