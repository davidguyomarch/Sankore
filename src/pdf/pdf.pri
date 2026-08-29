# Copyright (C) 2026 David Guyomarch

HEADERS      += src/pdf/GraphicsPDFItem.h \
                src/pdf/PDFRenderer.h \
                # src/pdf/UBWebPluginPDFWidget.h # Depends on XPDFRenderer (not available)
                # src/pdf/XPDFRenderer.h # XPDF library not available
                
SOURCES      += src/pdf/GraphicsPDFItem.cpp \
                src/pdf/PDFRenderer.cpp \
                # src/pdf/UBWebPluginPDFWidget.cpp # Depends on XPDFRenderer (not available)
                # src/pdf/XPDFRenderer.cpp # XPDF library not available
                          