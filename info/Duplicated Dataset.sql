-- create a duplicated dataset from pk
INSERT INTO album (filePath,fileName,icon,imageW,imageH,dateChanged,timeChanged,crc32,fileSize,keyWords,userComment,userDate)
SELECT  filePath,fileName, icon,imageW,imageH,dateChanged,timeChanged,crc32,fileSize,keyWords,userComment,userDate
FROM album WHERE pk = 8;

-- search duplicated datasets
SELECT pk, filePath, fileName, fileSize, crc32 FROM album
WHERE pk NOT IN 
(SELECT X.pk FROM album AS X GROUP BY X.filePath, X.fileName, X.fileSize, X.crc32 HAVING COUNT(*)=1);