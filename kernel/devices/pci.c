#include <types.h>
#include <system.h>
#include <irq.h>
#include <io.h>
#include <printk.h>
int nMethod1 = 0, nMethod2 = 0;
u32 g_nPCIMethod;
#define PCI_COMMAND	0x04	
#define PCI_COMMAND_MASTER		0x004
enum
{
	PCI_METHOD_1 = 0x01,
	PCI_METHOD_2 = 0x02,
	PCI_METHOD_BIOS = 0x04
};
int write_pci_config( int nBusNum, int nDevNum, int nFncNum, int nOffset, int nSize, u32 nValue )
{
	if ( 2 == nSize || 4 == nSize || 1 == nSize )
	{
		if ( g_nPCIMethod & PCI_METHOD_1 )
		{
			outportl(0x0cf8, 0x80000000 | ( nBusNum << 16 ) | ( nDevNum << 11 ) | ( nFncNum << 8 ) | ( nOffset & ~3 ) );
			switch ( nSize )
			{
			case 1:
				outportb( 0x0cfc + ( nOffset & 3 ),nValue );
				break;
			case 2:
				outportw( 0x0cfc + ( nOffset & 2 ),nValue );
				break;
			case 4:
				outportl(0x0cfc, nValue );
				break;
			default:
				return ( -1 );
			}
			return ( 0 );
		}
		else if ( g_nPCIMethod & PCI_METHOD_2 )
		{
			if ( nDevNum >= 16 )
			{
				printk("PCI: write_pci_config() with an invalid device number\n" );
			}
			outportb( 0x0cf8, 0xf0 | ( nFncNum << 1 )  );
			outportb(  0x0cfa,nBusNum );
			switch ( nSize )
			{
			case 1:
				outportb(( 0xc000 | ( nDevNum << 8 ) | nOffset ),nValue  );
				break;
			case 2:
				outportw(( 0xc000 | ( nDevNum << 8 ) | nOffset ),nValue );
				break;
			case 4:
				outportl( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) , nValue);
				break;
			default:
				return ( -1 );
			}
			outportb( 0x0cf8,0x0 );
			return ( 0 );
		}
		else
		{
			printk( "PCI: write_pci_config() called without PCI present\n" );
		}
	}
	else
	{
		printk( "PCI: Invalid size %d passed to write_pci_config()\n", nSize );
	}
	return ( -1 );
}

u32 read_pci_config( int nBusNum, int nDevNum, int nFncNum, int nOffset, int nSize )
{
	
	u32 nValue = 0;

	if ( 2 == nSize || 4 == nSize || 1 == nSize )
	{
		if ( g_nPCIMethod & PCI_METHOD_1 )
		{
		
			outportl( 0x0cf8, 0x80000000 | ( nBusNum << 16 ) | ( nDevNum << 11 ) | ( nFncNum << 8 ) | ( nOffset & ~3 ) );
			switch ( nSize )
			{
			case 1:
				nValue = inportb( 0x0cfc + ( nOffset & 3 ) );
				break;
			case 2:
				nValue = inportw( 0x0cfc + ( nOffset & 2 ) );
				break;
			case 4:
				nValue = inportl( 0x0cfc );
				break;
			default:
				break;
			}
			
			return ( nValue );
		}
		else if ( g_nPCIMethod & PCI_METHOD_2 )
		{
			if ( nDevNum >= 16 )
			{
				printk( "PCI: read_pci_config() with an invalid device number\n" );
			}
			
			outportb(0x0cf8, ( 0xf0 | ( nFncNum << 1 ) ) );
			outportb(0x0cfa,  nBusNum );
			switch ( nSize )
			{
			case 1:
				nValue = inportb( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			case 2:
				nValue = inportw( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			case 4:
				nValue = inportl( ( 0xc000 | ( nDevNum << 8 ) | nOffset ) );
				break;
			default:
				break;
			}
			outportb( 0x0cf8, 0x0 );
			
			return ( nValue );
		}
	
		else
		{
			printk( "PCI: read_pci_config() called without PCI present\n" );
		}
	}
	else
	{
		printk( "PCI: Invalid size %d passed to read_pci_config()\n", nSize );
	}
	return ( 0 );
}

void pci_inst_check(void)
{
	g_nPCIMethod = 0;
	
		/* Check PCI method 1 */
		outportl( 0x0cf8,0x80000000 );
		if ( inportl( 0x0cf8 ) == 0x80000000 )
		{
			outportl(0x0cf8, 0x0 );
			if ( inportl( 0x0cf8 ) == 0x0 )
			{
				g_nPCIMethod = PCI_METHOD_1;
				goto done;
			}
		}

		/* Check PCI method 2 */
		outportb( 0x0cf8, 0x0 );
		outportb( 0x0cfa , 0x0);
		if ( inportb( 0x0cf8 ) == 0x0 && inportb( 0x0cfa ) == 0x0 )
			g_nPCIMethod = PCI_METHOD_2;
	

done:
	switch( g_nPCIMethod )
	{
		case PCI_METHOD_1:
		{
			printk( "PCI: Using access method 1\n" );
			break;
		}

		case PCI_METHOD_2:
		{
			printk(  "PCI: Using access method 2\n" );
			break;
		}


		default:
		{
			printk(  "PCI: No PCI bus found\n" );
			break;
		}
	}

	return;
}
void enable_pci_master( int nBusNum, int nDevNum, int nFncNum )
{
  write_pci_config( nBusNum, nDevNum, nFncNum, PCI_COMMAND, 2,
                    read_pci_config( nBusNum, nDevNum, nFncNum, PCI_COMMAND, 2 )
                    | PCI_COMMAND_MASTER);
}
