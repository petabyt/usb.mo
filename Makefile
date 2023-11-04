MODULE_NAME=ptpview
MODULE_OBJS=ptpview.o lv.o chdk.o
MODULE_CFLAGS+=-O4

-include ../Makefile.modules

MY_ML_DIR=/home/$(USER)/Documents/magiclantern
test:
	rm -rf $(MY_ML_DIR)/modules/$(MODULE_NAME)
	cp -r . $(MY_ML_DIR)/modules/$(MODULE_NAME)
	cd $(MY_ML_DIR)/modules/$(MODULE_NAME) && make install
	cp $(MY_ML_DIR)/modules/$(MODULE_NAME)/$(MODULE_NAME).mo $(PWD)
