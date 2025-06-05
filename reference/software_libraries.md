# Software Library License Summary

## 1. NumPy
- **Description**: NumPy is the fundamental package for scientific computing with Python.
- **Discussion Approval**: [\[Ed (25) ENGG3800 - 2025 Sem 1 – Ed Discussion\]](https://edstem.org/au/courses/22191/discussion/2689305)
- **Website**: [https://github.com/numpy/numpy](https://github.com/numpy/numpy)
- **License**: BSD 3-Clause License ([License Link](https://github.com/numpy/numpy/blob/main/LICENSE.txt))
- **Implications**: The BSD 3-Clause License allows use in both open and closed-source projects with minimal restrictions. We must include the license and copyright.

## 2. PyQt5
- **Description**: PyQt5 is a set of Python bindings for the Qt application framework, enabling the development of cross-platform desktop applications with a modern graphical user interface.
- **Discussion Approval**: https://edstem.org/au/courses/22191/discussion/2484918
- **Website**: [https://www.riverbankcomputing.com/software/pyqt/](https://www.riverbankcomputing.com/software/pyqt/)
- **License**: GPL v3 ([License Link](https://www.riverbankcomputing.com/static/Docs/PyQt5/license.html))
- **Implications**: The GPL license requires that if our product is distributed, the full source code must also be made available under the same license terms. This makes PyQt5 incompatible with proprietary software unless a commercial license is obtained.

## 3. pyqtgraph
- **Description**: pyqtgraph is a pure-Python graphics and GUI library built on PyQt5/PySide2 and NumPy.
- **Discussion Approval**: https://edstem.org/au/courses/22191/discussion/2484921
- **Website**: [https://github.com/pyqtgraph/pyqtgraph](https://github.com/pyqtgraph/pyqtgraph)
- **License**: MIT License ([License Link](https://github.com/pyqtgraph/pyqtgraph/blob/master/LICENSE.txt))
- **Implications**: The MIT license is very permissive. It allows commercial use with no copyleft requirement, but we must include the license and attribution in documentation.

## 4. pyserial
- **Description**: pyserial encapsulates access to the serial port for Python, providing backends for Windows, Linux, BSD, Jython, and IronPython.
- **Discussion Approval**: [\[Ed (25) ENGG3800 - 2025 Sem 1 – Ed Discussion\]](https://edstem.org/au/courses/22191/discussion/2510604)
- **Website**: [https://github.com/pyserial/pyserial](https://github.com/pyserial/pyserial)
- **License**: BSD-style License ([License Link](https://github.com/pyserial/pyserial/blob/master/LICENSE.txt))
- **Implications**: This license permits integration into commercial products without requiring source code disclosure. We must retain the license and attribution.

## 5. pywin32
- **Description**: pywin32 provides access to many Windows APIs from Python, including COM support, registry access, event logging, and GUI operations.
- **Discussion Approval**: [\[Ed (25) ENGG3800 - 2025 Sem 1 – Ed Discussion\]](https://edstem.org/au/courses/22191/discussion/2704853)
- **Website**: [https://github.com/mhammond/pywin32](https://github.com/mhammond/pywin32)
- **License**:
  - Core modules: Python Software Foundation License v2.0 ([License Link](https://github.com/mhammond/pywin32/blob/main/win32/License.txt))
  - COM extensions: LGPL v2.1 ([License Link](https://github.com/mhammond/pywin32/blob/main/com/License.txt))
- **Implications**: The PSF License is permissive. The LGPL for COM components means that if they are modified and distributed, source code must be made available. Linking is allowed in proprietary software if not modified.

## 6. U8g2
- **Description**: U8g2 is a library for monochrome displays – primarily for AVR, requiring porting for STM32.
- **Discussion Approval**: [\[Ed (25) ENGG3800 - 2025 Sem 1 – Ed Discussion\]](https://edstem.org/au/courses/22191/discussion/2492612)
- **Website**: [https://github.com/olikraus/u8g2](https://github.com/olikraus/u8g2)
- **License**: BSD 2-Clause License ([License Link](https://github.com/olikraus/u8g2/blob/master/LICENSE))
- **Implications**: This license allows use and redistribution in both open-source and proprietary products, with minimal requirements such as retaining the license and attribution.

## 7. FatFs
- **Description**: FatFs is a generic FAT/exFAT filesystem module for small embedded systems.
- **Discussion Approval**: [\[Ed (25) ENGG3800 - 2025 Sem 1 – Ed Discussion\]](https://edstem.org/au/courses/22191/discussion/2484155)
- **Website**: [http://elm-chan.org/fsw/ff/00index_e.html](http://elm-chan.org/fsw/ff/00index_e.html)
- **License**: FatFs License (permissive, BSD-like) ([License Link](http://elm-chan.org/fsw/ff/arc/ff19c.zip))
- **Implications**: FatFs can be freely used in both open and proprietary projects. We are allowed to modify and redistribute the code without obligation to release source, provided credit to the author is retained in documentation.
