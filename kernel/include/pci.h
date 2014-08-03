#ifndef PCI_H
#define PCI_H
extern void pci_inst_check(void);
extern void enable_pci_master( int nBusNum, int nDevNum, int nFncNum );
#endif
