#include  <Uefi.h>
#include  <Library/UefiBootServicesTableLib.h>
#include  <Library/DevicePathLib.h>
#include  <Library/MemoryAllocationLib.h>
#include  <Library/ShellCEntryLib.h>
#include  <Library/ShellLib.h>
#include  <Library/UefiLib.h>
#include  <Protocol/LoadedImage.h>
#include  <Protocol/LoadFile.h>

#include  "elf_common.h"
#include  "elf64.h"

#define CheckStatus(Status, Code)	{\
	if(EFI_ERROR(Status)){\
		Print(L"Error: Status = %d, LINE=%d in %s\n", (Status), __LINE__, __func__);\
		Code;\
	}\
}

EFI_STATUS
EFIAPI
LoadFileByName (
	IN  CONST CHAR16	*FileName,
	OUT UINT8			**FileData,
	OUT UINTN			*FileSize
	)
{
	EFI_STATUS					Status;
	SHELL_FILE_HANDLE			FileHandle;
	EFI_FILE_INFO				*Info;
	UINTN						Size;
	UINT8						*Data;

	// Open File by shell protocol
	Status = ShellOpenFileByName(FileName, &FileHandle, EFI_FILE_MODE_READ, 0);
	CheckStatus(Status, return(Status));

	// Get File Info
	Info = ShellGetFileInfo(FileHandle);
	Size = (UINTN)Info->FileSize;
	FreePool(Info);

	// Allocate buffer to read file.
	// 'Runtime' so we can access it after ExitBootServices().
	Data = AllocateRuntimeZeroPool(Size);
	if(Data == NULL){
		Print(L"Error: AllocateRuntimeZeroPool failed\n");
		return(EFI_OUT_OF_RESOURCES);
	}

	// Read file into Buffer
	Status = ShellReadFile(FileHandle, &Size, Data);
	CheckStatus(Status, return(Status));

	// Close file
	Status = ShellCloseFile(&FileHandle);
	CheckStatus(Status, return(Status));

	*FileSize = Size;
	*FileData = Data;

	return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
ElfLoadSegment (
	IN  CONST VOID	*ElfImage,
	OUT VOID		**EntryPoint
	)
{
	Elf64_Ehdr	*ElfHdr;
	UINT8		*ProgramHdr;
	Elf64_Phdr	*ProgramHdrPtr;
	UINTN		Index;

	ElfHdr = (Elf64_Ehdr *)ElfImage;
	ProgramHdr = (UINT8 *)ElfImage + ElfHdr->e_phoff;

	// Load every loadable ELF segment into memory
	for(Index = 0; Index < ElfHdr->e_phnum; Index++){
		ProgramHdrPtr = (Elf64_Phdr *)ProgramHdr;

		// Only consider PT_LOAD type segments
		if(ProgramHdrPtr->p_type == PT_LOAD){
			VOID	*FileSegment;
			VOID	*MemSegment;
			VOID	*ExtraZeroes;
			UINTN	ExtraZeroesCount;

			// Load the segment in memory
			FileSegment = (VOID *)((UINTN)ElfImage + ProgramHdrPtr->p_offset);
			MemSegment = (VOID *)ProgramHdrPtr->p_vaddr;
			gBS->CopyMem(MemSegment, FileSegment, ProgramHdrPtr->p_filesz);
		
			// Fill memory with zero for .bss section and ...
			ExtraZeroes = (UINT8 *)MemSegment + ProgramHdrPtr->p_filesz;
			ExtraZeroesCount = ProgramHdrPtr->p_memsz - ProgramHdrPtr->p_filesz;
			if(ExtraZeroesCount > 0){
				gBS->SetMem(ExtraZeroes, 0x00, ExtraZeroesCount);
			}
		}

		// Get next program header
		ProgramHdr += ElfHdr->e_phentsize;
	}

	*EntryPoint = (VOID *)ElfHdr->e_entry;

	return (EFI_SUCCESS);
}

EFI_STATUS
EFIAPI
StartUefiAppByName (
	IN  CONST CHAR16	*FileName,
	OUT INTN			*ReturnValue
	)
{
	EFI_STATUS					Status;
	EFI_DEVICE_PATH_PROTOCOL	*FilePath = NULL;
	EFI_HANDLE					TargetAppHandle = NULL;
	EFI_HANDLE					*FileSystemHandles = NULL;
	UINTN						NumberFileSystemHandles = 0;
	CHAR16						*FilePathText = NULL;

	// Get simple file system handles
	Status = gBS->LocateHandleBuffer(
			ByProtocol,
			&gEfiSimpleFileSystemProtocolGuid,
			NULL,
			&NumberFileSystemHandles,
			&FileSystemHandles
			);
	if(EFI_ERROR(Status)){
		Print(L"Error: LocateHandleBuffer failed\n");
		return(-1);
	}

	// Get device path of the file
	FilePath = FileDevicePath(FileSystemHandles[0], FileName);
	if(FilePath == NULL){
		Print(L"Error: DeviceFilePath\n");
		return(-1);
	}

	if((FilePathText = ConvertDevicePathToText(FilePath, FALSE, FALSE)) != NULL){
		Print(L"FilePath = %s\n", FilePathText);
	}

	// Load pe/coff image of the file into memory
	Status = gBS->LoadImage(
			TRUE,
			gImageHandle,
			FilePath,
			NULL,
			0,
			&TargetAppHandle);

	if(EFI_ERROR(Status)){
		Print(L"Error: LoadImage\n");
		return(-1);
	}
	Print(L"TargetAppHandle = 0x%x\n", TargetAppHandle);

	// Execute the image
	Status = gBS->StartImage(TargetAppHandle, 0, 0);
	Print(L"StartImage, Return Value = %d(0x%x)\n", Status, Status);

	if(ReturnValue != NULL){
		*ReturnValue = Status;
	}

	return(EFI_SUCCESS);
}


INTN
EFIAPI
ShellAppMain (
  IN UINTN Argc,
  IN CHAR16 **Argv
  )
{
	EFI_STATUS		Status;
	CHAR16			*FileName = L"fs0:\\out-serial-A.elf";
	UINTN			FileSize;
	UINT8			*FileData;
	VOID			*EntryPoint = NULL;

	if(2 <= Argc){
		FileName = Argv[1];
	}

	// Load the ELF file to buffer
	Print(L"Load the file = %s\n", FileName);
	Status = LoadFileByName(FileName, &FileData, &FileSize);
	CheckStatus(Status, return(-1));

	// Load the program section(PT_LOAD) of the ELF executable into memory
	Print(L"Load the program section of the ELF executable into memory\n");
	Status = ElfLoadSegment(FileData, &EntryPoint);
	CheckStatus(Status, return(-1));

	// ExitBootServices
	
	// Jump to the entrypoint of executable
	Print(L"Execute the program\n");
	goto *EntryPoint;

	Print(L"jump addres = %lx\n", *FileData);
	Print(L"%x %x %x %x %x %x %x %x\n",
			FileData[0], FileData[1], FileData[2], FileData[3], 
			FileData[4], FileData[5], FileData[6], FileData[7]);

	goto *(FileData);

	// Unreachable!!!
	return 0;

//	{
//		INTN ReturnValue = 0;
//		Status = StartUefiAppByName(FileName, &ReturnValue);
//		CheckStatus(Status, return(-1));
//	}
//
//	return 0;
}

