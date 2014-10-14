import os
import sys
import subprocess

css = open("kashyyyk.css", "w")
css.write("body {"
"align: center;"
"margin: 0px auto;"
"width: 50%;"
"color: #888899;"
"font-family: \"DejaVu sans\";"
"background: -webkit-linear-gradient(left,  black 0px, #222233  calc(50% - ( 928px / 2 ) ), #222233  calc( 50% + ( 928px / 2 ) ), black 100%);"
"background: -o-linear-gradient(     right, black 0px, #222233  calc(50% - ( 928px / 2 ) ), #222233  calc( 50% + ( 928px / 2 ) ), black 100%);"
"background: -moz-linear-gradient(   right, black 0px, #222233  calc(50% - ( 928px / 2 ) ), #222233  calc( 50% + ( 928px / 2 ) ), black 100%);"
"background: linear-gradient(     to right, black 0px, #222233  calc(50% - ( 928px / 2 ) ), #222233  calc( 50% + ( 928px / 2 ) ), black 100%);"
"}"
"\n"
"h1 {"
"box-shadow: 10px 5px 5px #555577;"
"color: black;"
"padding-left: 1em;"
"padding-right: 1em;"
"padding-top: 0.5em;"
"padding-bottom: 0.25em;"
"border-color: #666666;"
"background: #AAAAAA;"
"border-radius:8px;"
"border-color: #220202;"
"}"
"\n"
"h2 {"
"box-shadow: 5px 2px 2px #555577;"
"}")

css.close()

def generate():
  try:
    subprocess.call(["pandoc", "--reference-links", "README.md", "--css=kashyyyk.css", "-o", "kashyyyk.html"])
    subprocess.call(["pandoc", "--reference-links", "license.txt", "--css=kashyyyk.css",  "-o", "license.html"])
  except OSError as e:
    print "pandoc was not installed. Not generating HTML readme."

