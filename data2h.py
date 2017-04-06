import gzip
import shutil
import sys
import binascii
import cStringIO
import io
import os



def convert_file(inputfilename, outputfilename):
    try:
        filename = outputfilename
        inputfile = open(inputfilename, "rb")
        outputfile = open(outputfilename, "w")
        binary = inputfile.read()
        if len(binary) > 0:
            #outputfile.write('static const char PROGMEM ')  ## used with ESP8266
            outputfile.write('static const char ') ## ESP32

            index = filename.rfind('/')
            if index == -1:
                index = filename.rfind('\\')
            if index >= 0:
                filename = filename[index+1:]
            outputfile.write(filename.replace('.', '_'))
            outputfile.write('[')
            outputfile.write(str(len(binary)))
            outputfile.write('] = {\n')

            if sys.version_info[0] >= 3:
                text = str(binascii.hexlify(binary), 'UTF-8')
            else:
                text = str(binascii.hexlify(binary))

            outputfile.write('0x')
            outputfile.write(text[0:2])
            i = 2
            while i < len(text):
                outputfile.write(', ')
                if i % 100 == 0:
                    outputfile.write('\n')
                outputfile.write('0x')
                outputfile.write(text[i:i+2])
                i += 2
            outputfile.write('\n};\n\n')
            outputfile.close()
            inputfile.close()
            print("converted: " + inputfilename + " --> " + outputfilename)


    except IOError:
        print("cannot open input file %s" % inputfilename)
        outputfile.close()

try: 
    src = open("data/index.html", "rb") 
    gzf = gzip.GzipFile(filename="indexhtml.gz", mode='wb')
    gzf.write(src.read())
    src.close()
    gzf.close()
    
except IOError:
    print("cannot open index.html for reading or indexhtml.gz for writing")

try:
    convert_file("indexhtml.gz", 'main/indexhtml.h')
    #convert_file('data/nice.wav', 'main/wavdata.h')
    convert_file('data/font.woff', 'main/fontwoff.h')    
    convert_file('data/font.ttf', 'main/fontttf.h')

except IOError:
    print("cannot open indexhtml.h or font*.h for writing, or cannot find file to convert")

#cleanup
os.remove("indexhtml.gz")

