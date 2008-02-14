
// NOTE for bit string/field addressing
// ************************************
// modDim must be passed as 10 for bit string instructions,
// and as 11 for bit field instructions



// Output variables for ReadAMAddress()
static UINT8 amFlag;
static UINT32 amOut;
static UINT32 bamOffset;

// Appo temp var
static UINT32 amLength1,amLength2;


// Global vars used by AM functions
static UINT32 modAdd;
static UINT8 modM;
static UINT8 modVal;
static UINT8 modVal2;
static UINT8 modWriteValB;
static UINT16 modWriteValH;
static UINT32 modWriteValW;
static UINT8 modDim;

// Addressing mode functions and tables
#include "am1.c" // ReadAM
#include "am2.c" // ReadAMAddress
#include "am3.c" // WriteAM

/*
  Input:
  modAdd
    modDim

  Output:
    amOut
    amLength
*/

static UINT32 ReadAM(void)
{
	modM=modM?1:0;
	modVal=OpRead8(modAdd);
	return AMTable1[modM][modVal>>5]();
}

static UINT32 BitReadAM(void)
{
	modM=modM?1:0;
	modVal=OpRead8(modAdd);
	return BAMTable1[modM][modVal>>5]();
}



/*
  Input:
  modAdd
    modDim

  Output:
    amOut
    amFlag
    amLength
*/

static UINT32 ReadAMAddress(void)
{
	modM=modM?1:0;
	modVal=OpRead8(modAdd);
	return AMTable2[modM][modVal>>5]();
}

static UINT32 BitReadAMAddress(void)
{
	modM=modM?1:0;
	modVal=OpRead8(modAdd);
	return BAMTable2[modM][modVal>>5]();
}

/*
  Input:
  modAdd
    modDim
    modWriteValB/H/W

  Output:
    amOut
    amLength
*/

static UINT32 WriteAM(void)
{
	modM=modM?1:0;
	modVal=OpRead8(modAdd);
	return AMTable3[modM][modVal>>5]();
}


