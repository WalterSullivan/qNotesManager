    qNotesManager

     1. About
       qNotesManager is an simple yet powerful note-taking software. If you 
       like to keep your notes organized like I do, then this software is for
       you.
       Main features:
        - Format note text in WYSIWYG-editor;
        - Insert images, lists, tables;
        - Add tags to your notes;
        - Group notes in categories (folders);
        - Filter notes by their creation / modification time;
        - Search in note text or in entire document;
        - Attach files to notes;
        - Decorate notes with different colors and icons;
        - Add bookmarks to frequently used notes for easy access;
        - Save file may be encrypted if you want to keep your information in
          secret;
        - Quick note function: create a new note with text from clipboard with
          just one click.

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

    3. Platforms
       qNotesManager may be built on following platforms:
       - MS Windows;
       - GNU/Linux;
       - OS/2 (thanks to Elbert Pol for OS/2 build);
       - Mac OS (maybe, not tested).

       Dependencies:
       - Qt (>=4.6)
       - openssl

       Building on Windows 32:
       qNotesManager requires openssl library to build. Make sure you have 
       OPENSSL_ROOT_DIR system variable defined or change OPENSSLPATH local 
       variable in qNotesManager.pro file. You can find precompiled
       openssl libraries at 
       http://www.npcglib.org/~stathis/blog/precompiled-openssl/.

       Building on Linux:
       Install required libraries with your package manager.

       Building on Mac:
       I have no possibility to test this software on Mac. However Qt is a
       crossplatform framework and it should work fine. If you are a Mac
       developer and want to help, please contact me.

       To build this program:

       1. run qmake
       2. run make
