#!/bin/bash

# WE HAVE A TABLE IN OUR DATABASE WHERE WE INSERT DESCRIPTIONS FOR EVERY ARTIFACT OUR MUSEUM SHOWS TO THE PUBLIC
# EVERY DESCRIPTION AS A DIFFERENT LENGTH DEPENDING ON THE KIND OF USER
# SINCE MYSQL IS A PAIN WHEN IT COMES TO INSERT LONG TEXT (ESCAPING INCLUDED), I WROTE THIS SIMPLE SCRIPT
# MODIFY THIS SCRIPT AS YOU LIKE

# INSERT HERE YOUR DATA (WHEN THE PROGRAM RUNS, YOU WILL BE ASKED TO INSERT YOUR PASSWORD TO GET ACCESS)
DB_USER=""
DB_NAME=""
TABLE_NAME=""

# ARTIFACT ID_CODE (IT IS A PRIMARY KEY)
id=""

# DESCRPTION 1: NORMAL USER
n_description=""

# DESCRIPTION 2: INTERMEDIATE USER
y_description=""

# DESCRIPTION 3: EXPERT USER
e_description=""

# THE AREA IN WHICH THE ARTIFACT IS LOCATED
area=""

# HERE ESCAPE CHARACTERS THAT MYSQL DOES NOT LIKE, YOU CAN EXTEND IT
escaped_valore1=$(echo "$n_description" | sed -e "s/(/\\(/g" -e "s/)/\\)/g" -e "s/'/\\\\'/g" -e 's/"/\\"/g')
escaped_valore2=$(echo "$y_description" | sed -e "s/(/\\(/g" -e "s/)/\\)/g" -e "s/'/\\\\'/g" -e 's/"/\\"/g')
escaped_valore3=$(echo "$e_description" | sed -e "s/(/\\(/g" -e "s/)/\\)/g" -e "s/'/\\\\'/g" -e 's/"/\\"/g')

# INSERT INTO A TABLE
INSERT_QUERY="INSERT INTO $TABLE_NAME (artifact_id, n_description, y_description, e_description, area)
    VALUES ('$id', '$escaped_valore1', '$escaped_valore2', '$escaped_valore3', '$area');"

mysql -u $DB_USER -p $DB_NAME -e "$INSERT_QUERY"

if [ $? -eq 0 ]; then
    echo "[OK] RECORD INSERTED SUCCESSFULLY."
else
    echo "[FAIL] FAILED TO INSERT RECORD."
fi