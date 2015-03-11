task :default do
	puts '=== How to build ==='
	puts 'rake setup'
	puts 'rake build'
	puts '=== How to run ==='
	puts 'rake run'
end

task :setup do
	# Setup EDKII
	if !FileTest.symlink?('edk2/AppPkg/Applications/Hello') then
        sh 'mv edk2/AppPkg/Applications/Hello edk2/AppPkg/Applications/_Hello'
        sh 'ln -sf ../../../src edk2/AppPkg/Applications/Hello'
	end

    sh 'make -C edk2/BaseTools'

	sh "$SHELL edksetup.sh", {chdir: 'edk2'}, {}
	# bash-like shell is needed ('/bin/sh' is not working)

    sh 'sed -i -e "s/= Nt32Pkg\/Nt32Pkg.dsc/= AppPkg\/AppPkg.dsc/g" edk2/Conf/target.txt'
    sh 'sed -i -e "s/= IA32/= X64/g" edk2/Conf/target.txt'
    sh 'sed -i -e "s/= DEBUG/= RELEASE/g" edk2/Conf/target.txt'
    sh 'sed -i -e "s/= MYTOOLS/= GCC49/g" edk2/Conf/target.txt'
    
	sh 'mkdir -p build'

	# Setup emuration environment
    sh 'wget http://sourceforge.net/projects/edk2/files/OVMF/OVMF-X64-r15214.zip'
    sh 'unzip OVMF-X64-r15214.zip OVMF.fd'
    sh 'rm OVMF-X64-r15214.zip'
    sh 'mkdir -p image/EFI/BOOT'
    sh 'ln -sf ../../../bin/UefiOSLoader.efi image/EFI/BOOT/UefiOSLoader.efi'
end

task :build do
    sh 'build', {chdir: 'edk2'}, {}
	sh 'cp edk2/Build/AppPkg/RELEASE_GCC49/X64/AppPkg/Applications/Hello/Hello/OUTPUT/UefiOSLoader.efi bin/'
end

task :run do
	sh 'qemu-system-x86_64 -nographic -bios OVMF.fd -hda fat:image -net none'
end

task :example => [:setup, :build] do
	sh './make.sh', {chdir: 'examples'}, {}
	Rake::Task["run"].invoke()
end

task :clean do
    sh 'rm -rf bin'
end

