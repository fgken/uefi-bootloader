if [ "`readlink edk2/AppPkg/Applications/Hello`" != "../../../src" ]; then
	mv edk2/AppPkg/Applications/Hello edk2/AppPkg/Applications/_Hello
	ln -sf ../../../src edk2/AppPkg/Applications/Hello
fi

cd edk2

make -C BaseTools

. edksetup.sh
sed -i -e "s/= Nt32Pkg\/Nt32Pkg.dsc/= AppPkg\/AppPkg.dsc/g" Conf/target.txt
sed -i -e "s/= IA32/= X64/g" Conf/target.txt
sed -i -e "s/= DEBUG/= RELEASE/g" Conf/target.txt
sed -i -e "s/= MYTOOLS/= GCC49/g" Conf/target.txt

build

cd ..

mkdir -p bin
cp edk2/Build/AppPkg/RELEASE_GCC49/X64/AppPkg/Applications/Hello/Hello/OUTPUT/UefiOSLoader.efi bin/

