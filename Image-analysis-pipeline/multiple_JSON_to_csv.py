#Create csv dataset from folder of JSON files developed using LabelMe 

#dependencies 
from shapely.geometry import Polygon
import pandas as pd
from pandas.io.json import json_normalize
import os,json

#path to folder containing all json files to combine into dataframe (ensure there are no additional JSON files)
path_to_json = 'c:/path/name/' #add full path to that directory 

#import json files and add each to a list called All and then data 
appended_data = []
for file_name in [file for file in os.listdir(path_to_json) if file.endswith('.json')]:
  with open(path_to_json + file_name) as json_file:
        All = json.load(json_file)
        data = [All]
        #create data frames
        df = pd.DataFrame.from_dict(pd.json_normalize(data, 'shapes'), orient='columns')
        #label with image name 
        df['image_path'] = All['imagePath']
        appended_data.append(df)        
#create a dataframe from all the appended data 
New_df = pd.concat(appended_data)

#split into two dataframes to get areas from polygons
area = New_df[New_df['shape_type']=='polygon']
point = New_df[New_df['shape_type']=='point']


area['Polygon'] = area['points'].apply(Polygon)  #create polygons
area['Pixel_area'] = area.apply(lambda row: row.Polygon.area, axis=1) #calculate area of polygon in pixels
area['CM_area'] = area['Pixel_area']/156025 #convert to cm (1cm = 395pixels)

all_data = pd.concat([area, point], axis=0) #recombine dataframes

#split image path to get date_time as its own column 
all_data['date']=all_data['image_path'].str.slice(stop=13)
all_data['image_path']=all_data['image_path'].str.slice(start=20)

#to export: 
all_data.to_csv(r'c:/path/name/filename.csv')


#or, uncomment below to export directly to excel
# create excel writer
# writer = pd.ExcelWriter('test1.xlsx') 
# # write dataframe to excel sheet named 'Doc name here'
# New_df.to_excel(writer, 'test1')
# # save the excel file
# writer.save()

print('done') #just so you know it happened 

