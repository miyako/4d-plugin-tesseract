4d-plugin-tesseract
===================

4D implementation of Tesseract OCR 3.2.

You can support more languages by adding files from https://github.com/tesseract-ocr/langdata.

### Platform

| carbon | cocoa | win32 | win64 |
|:------:|:-----:|:---------:|:---------:|
|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|<img src="https://cloud.githubusercontent.com/assets/1725068/22371562/1b091f0a-e4db-11e6-8458-8653954a7cce.png" width="24" height="24" />|

### Version

<img src="https://cloud.githubusercontent.com/assets/1725068/18940649/21945000-8645-11e6-86ed-4a0f800e5a73.png" width="32" height="32" /> <img src="https://cloud.githubusercontent.com/assets/1725068/18940648/2192ddba-8645-11e6-864d-6d5692d55717.png" width="32" height="32" />

### Releases

[1.1](https://github.com/miyako/4d-plugin-tesseract/releases/tag/1.1)

## Syntax

```
path:=OCR Get language folder
success:=OCR Set language folder (path)
```

Parameter|Type|Description
------------|------------|----
path|TEXT|
success|LONGINT|

```
success:=OCR Read image text (path;text;language)
```

Parameter|Type|Description
------------|------------|----
path|TEXT|
text|TEXT|
language|TEXT|
success|LONGINT|

```
timeout:=OCR Get timeout
OCR SET TIMEOUT (timeout)
```

Parameter|Type|Description
------------|------------|----
timeout|LONGINT|default: ``15`` seconds
