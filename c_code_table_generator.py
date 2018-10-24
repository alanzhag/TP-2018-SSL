#!/usr/bin/env python2
# -*- coding: utf-8 -*-
"""
Created on Tue Oct 23 20:12:36 2018

@author: alanzhao
"""

import pandas as pd
from string import Template

df = pd.read_html("table.html", header=0)[0]
number_of_columns = len(list(df))
_converters = {key: str for key in list(range(number_of_columns))}

df = pd.read_html("table.html", header=0, converters=_converters, keep_default_na=False)
df = df[0]

code_line = Template("automatonTable[$row][$col] = state$sid;")
row_line = Template("\n\t//ROW $index $row")

dimensions = df.shape

for index, row in df.iterrows():
    print row_line.substitute(index=index, row=list(row)[1:])
    for sub_index, data in enumerate(row[1:]):
        _row = index
        _col = sub_index
        _sid = data
        print code_line.substitute(row=_row, col=_col, sid=_sid)
