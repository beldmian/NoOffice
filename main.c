#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdbool.h>

const char* DOCUMENT_FORMAT = "<?xml version='1.0' encoding='UTF-8' standalone='yes'?><w:document xmlns:wpc=\"http://schemas.microsoft.com/office/word/2010/wordprocessingCanvas\" xmlns:mo=\"http://schemas.microsoft.com/office/mac/office/2008/main\" xmlns:mc=\"http://schemas.openxmlformats.org/markup-compatibility/2006\" xmlns:mv=\"urn:schemas-microsoft-com:mac:vml\" xmlns:o=\"urn:schemas-microsoft-com:office:office\" xmlns:r=\"http://schemas.openxmlformats.org/officeDocument/2006/relationships\" xmlns:m=\"http://schemas.openxmlformats.org/officeDocument/2006/math\" xmlns:v=\"urn:schemas-microsoft-com:vml\" xmlns:wp14=\"http://schemas.microsoft.com/office/word/2010/wordprocessingDrawing\" xmlns:wp=\"http://schemas.openxmlformats.org/drawingml/2006/wordprocessingDrawing\" xmlns:w10=\"urn:schemas-microsoft-com:office:word\" xmlns:w=\"http://schemas.openxmlformats.org/wordprocessingml/2006/main\" xmlns:w14=\"http://schemas.microsoft.com/office/word/2010/wordml\" xmlns:wpg=\"http://schemas.microsoft.com/office/word/2010/wordprocessingGroup\" xmlns:wpi=\"http://schemas.microsoft.com/office/word/2010/wordprocessingInk\" xmlns:wne=\"http://schemas.microsoft.com/office/word/2006/wordml\" xmlns:wps=\"http://schemas.microsoft.com/office/word/2010/wordprocessingShape\" mc:Ignorable=\"w14 wp14\"><w:body><w:p w:rsidR=\"005F670F\" w:rsidRDefault=\"005F79F5\">%s</w:p></w:body><w:sectPr w:rsidR=\"005F670F\"><w:pgSz w:w=\"12240\" w:h=\"15840\"/><w:pgMar w:top=\"1440\" w:right=\"1440\" w:bottom=\"1440\" w:left=\"1440\" w:header=\"720\" w:footer=\"720\" w:gutter=\"0\"/><w:cols w:space=\"720\"/><w:docGrid w:linePitch=\"360\"/></w:sectPr></w:document>"; 

bool prefix(const char *pre, const char *str)
{
	return strncmp(pre, str, strlen(pre)) == 0;
}

char* slice_str(char* str, int start, int end) {
	char* out = calloc(end-start, sizeof(char));
	for (int i = start; i < end; i++) {
		out[i-start] = str[i];
	}
	return out;
}

void concat_str(char **str, const char *str2) {
    char *tmp = NULL;

    if ( *str != NULL && str2 == NULL ) {
        free(*str);
        *str = NULL;
        return;
    }
    if (*str == NULL) {
        *str = calloc( strlen(str2)+1, sizeof(char) );
        memcpy( *str, str2, strlen(str2) );
    }
    else { 
        tmp = calloc( strlen(*str)+1, sizeof(char) );
        memcpy( tmp, *str, strlen(*str) );
        *str = calloc( strlen(*str)+strlen(str2)+1, sizeof(char) );
        memcpy( *str, tmp, strlen(tmp) );
        memcpy( *str + strlen(*str), str2, strlen(str2) );
        free(tmp);
    }

}

int main()
{
	// Correct file opening
	FILE *md;
	errno_t err = fopen_s(&md, "document.md", "r");
	if (err != 0) {
		printf("Error opening document.md");
		return 1;
	}

	// Geting file stats
	struct stat sb;
	stat("document.md", &sb);

	// Allocate memory
	char *file_contents = malloc(sb.st_size);
	char **lines = malloc(sb.st_size);

	// Iterate through lines
	int line_number = 0;
	while (fscanf_s(md, "%[^\n] ", file_contents) != EOF) {
		int llen = strlen(file_contents);
		lines[line_number] = calloc(llen, sizeof(char));
		strcpy(lines[line_number], file_contents);
		line_number ++;
	}
	fclose(md);

	char* document = "\0";
	for (int i = 0; i < line_number; i++) {
		char* line = lines[i];
		int lline = strlen(line);
		// Parse markdown
		if (prefix("# ", lines[i])) {
			const int format_size = 76;
			const char* format = "<w:p><w:pPr><w:pStyle w:val=\"Heading1\"/></w:pPr><w:r><w:t>%s</w:t></w:r></w:p>";
			char* doc_line = malloc((lline+format_size) * sizeof(char));
			sprintf(doc_line, format, slice_str(line, 2, lline));
			concat_str(&document, doc_line);
		}
		else {
			const int format_size = 22;
			const char* format = "<w:r><w:t>%s</w:t></w:r>";
			char* doc_line = malloc((lline+format_size) * sizeof(char));
			sprintf(doc_line, format, line);
			concat_str(&document, doc_line);
		
		}
	}
	
	system("unzip format.docx -d format");

	FILE *out;
	char *filepath = "./format/word/document.xml";
	remove(filepath);
	err = fopen_s(&out, filepath, "w");
	if (err != 0) {
		printf("Error opening document.xml");
		return 1;
	}

	fprintf(out, DOCUMENT_FORMAT, document);
	fclose(out);
	
	system("cd format && zip -r ../out.docx .");
	return 0;
}
