    qNotesManager

     1. About
       qNotesManager is an simple note-taking software.
       What you can do (basic features):
       - Format note text in WYSIWYG-editor;
	   - Insert images, lists, tables;
	   - Add tags to your notes;
	   - Group yout notes in categories (folders);
	   - Filter notes by their creation / modification time;
	   - Save file may be encrypted if you want to keep your information in
	   secret;
	   - Quick note function: create a new note with text from clipboard with
	   just one click;
	   - And more.

       Author: Yury Khamenkov
       waltersullivan.11121@gmail.com
	   
	   qNotesManager is free and opensource software. Please, feel free to 
	   report bugs or even contribute at 
	   http://github.com/WalterSullivan/qNotesManager .
	   
	   Icons: Fugue Icons pack
	   (C) 2012 Yusuke Kamiyamane. All rights reserved.
	   These icons are licensed under a Creative Commons 
	   Attribution 3.0 License.
	   <http://creativecommons.org/licenses/by/3.0/>

     2. License

       This program is free software: you can redistribute it and/or modify
       it under the terms of the GNU General Public License as published by
       the Free Software Foundation, either version 3 of the License, or
       (at your option) any later version.

       This program is distributed in the hope that it will be useful,
       but WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
       GNU General Public License for more details.

       You should have received a copy of the GNU General Public License
       along with this program.  If not, see <http://www.gnu.org/licenses/>.

       See 'LICENSE' file to know more about GNU GPL.

    3. Dependencies
       This program requeries:
	   - Qt4 libraries (4.4 or above)
	   - openssl
	   - qca2
	   - qca-ossl2
	   
	   Building on Windows 32:
	   All required libraries are located in libs/win32 directory. To run 
	   executable, make sure excutable's directory contains these files:
	   -libeay32.dll
	   -ssleay32.dll
	   -qca2.dll
	   -crypto/qca-ossl2.dll
	   
	   Building on Linux:
	   Install required libraries with your package manager.
	   
	   Building on Mac:
	   I have no possibility to test this software on Mac. However Qt is a
	   crossplatform framework and it should work fine. If you are a Mac
	   developer and want to help, please contact me.

       To build this program:

       1. run qmake
       2. run make