import psycopg2
from psycopg2 import Error
import numpy as np
from scipy.io import savemat
import sys

try:
    # Connect to an existing database
    connection = psycopg2.connect(user="lsldb_user",
                                  password="azerty",
                                  host="localhost",
                                  port="5432",
                                  database="lsldb")

    # Create a cursor to perform database operations
    cursor = connection.cursor()
    
    table_name = sys.argv[1]
    uid = sys.argv[2]
    filename = "data_"+table_name+"_"+uid;

    print("Table name: ", table_name)
    print("UID: ", uid)
    print("Filename: ", filename)
    
    # Executing a SQL query
    cursor.execute("select time from "+table_name+" where uid='"+uid+"'")
    ind = cursor.fetchall();
    cursor.execute("select t from "+table_name+" where uid='"+uid+"'")
    time = cursor.fetchall();
    cursor.execute("select data from "+table_name+" where uid='"+uid+"'")
    values = cursor.fetchall();

    # Fetch result
    print("Fetching result")
    val = np.asarray(values)

    print("Saving to mat file")
    
    
    data = {'counter':ind,
            'time':time,
            'values':val[:,0]}
    savemat(filename+'.mat', data)
    

except (Exception, Error) as error:
    print( error)
finally:
    if (connection):
        cursor.close()
        connection.close()
