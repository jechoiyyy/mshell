#include <stdio.h>
#include <stdlib.h>
#include "print.h"

// t_filename 구조체 출력 함수
void print_filename(t_filename *filename, int index)
{
    if (!filename)
    {
        printf("    FILENAME[%d]: NULL\n", index);
        return;
    }
    
    printf("    FILENAME[%d]:\n", index);
    printf("      filename: %s\n", filename->filename ? filename->filename : "NULL");
    printf("      flag: %s\n", filename->flag ? "WRONG" : "CORRECT");
    printf("      append_mode: %d\n", filename->append_mode);
    printf("      hd: %d\n", filename->hd);
}

// t_file 리스트 출력 함수 (수정됨)
void print_file_list(t_file *head, char *str)
{
    t_file *current;
    int index;

    if (!head)
    {
        printf("  %s list: EMPTY\n", str);
        return;
    }
    
    printf("  %s list:\n", str);
    current = head;
    index = 0;
    
    while (current)
    {
        printf("  [%d] t_file node:\n", index);
        
        // input_file 출력
        if (current->input_file)
        {
            printf("    input_file:\n");
            print_filename(current->input_file, 0);
        }
        else
        {
            printf("    input_file: NULL\n");
        }
        
        // output_file 출력
        if (current->output_file)
        {
            printf("    output_file:\n");
            print_filename(current->output_file, 0);
        }
        else
        {
            printf("    output_file: NULL\n");
        }
        
        current = current->next;
        index++;
    }
}

// 단일 cmd 구조체 출력 함수
void print_cmd(t_cmd *cmd, int index)
{
    if (!cmd)
    {
        printf("CMD[%d]: NULL\n", index);
        return;
    }
    
    printf("=== CMD[%d] ===\n", index);
    
    // args 출력
    printf("  args: ");
    if (!cmd->args)
        printf("NULL\n");
    else
    {
        printf("[\n");
        for (int i = 0; cmd->args[i]; i++)
            printf("    [%d]: \"%s\"\n", i, cmd->args[i]);
        printf("  ]\n");
    }
    
    // file 리스트 출력 (수정됨)
    if (cmd->file)
        print_file_list(cmd->file, "file");
    else
        printf("  file list: NULL\n");
    
    // hd (파일 디스크립터) 출력
    printf("  hd (file descriptor): %d", cmd->hd);
    if (cmd->hd == -1)
        printf(" (INVALID/CLOSED)");
    else if (cmd->hd == 0)
        printf(" (STDIN)");
    else if (cmd->hd == 1)
        printf(" (STDOUT)");
    else if (cmd->hd == 2)
        printf(" (STDERR)");
    else if (cmd->hd > 2)
        printf(" (CUSTOM FD)");
    printf("\n");
    
    // next 포인터 상태
    printf("  next: %s\n", cmd->next ? "EXISTS" : "NULL");
    
    printf("\n");
}

// 전체 cmd 리스트 출력 함수
void print_cmd_list(t_cmd *head)
{
    if (!head)
    {
        printf("CMD LIST: EMPTY\n");
        return;
    }
    
    printf("========== CMD LIST DEBUG ==========\n");
    
    t_cmd *current = head;
    int index = 0;
    
    while (current)
    {
        print_cmd(current, index);
        current = current->next;
        index++;
    }
    
    printf("Total commands: %d\n", index);
    printf("====================================\n");
}

// 간단한 요약 출력 함수 (한 줄로) - 수정됨
void print_cmd_summary(t_cmd *head)
{
    if (!head)
    {
        printf("CMD SUMMARY: EMPTY\n");
        return;
    }
    
    printf("CMD SUMMARY: ");
    t_cmd *current = head;
    int index = 0;
    
    while (current)
    {
        printf("[%d]", index);
        if (current->args && current->args[0])
            printf("(%s)", current->args[0]);
        
        // t_file 리스트에서 입력/출력 파일 확인
        if (current->file)
        {
            t_file *file_node = current->file;
            while (file_node)
            {
                if (file_node->input_file)
                    printf("<%s", file_node->input_file->filename ? file_node->input_file->filename : "?");
                if (file_node->output_file)
                    printf(">%s", file_node->output_file->filename ? file_node->output_file->filename : "?");
                file_node = file_node->next;
            }
        }
        
        if (current->hd > 2)  // 표준 fd가 아닌 경우
            printf("[fd:%d]", current->hd);
        
        if (current->next)
            printf(" -> ");
        
        current = current->next;
        index++;
    }
    printf("\n");
}

// 파일 디스크립터만 따로 확인하는 함수
void print_cmd_fd_info(t_cmd *head)
{
    if (!head)
    {
        printf("FD INFO: EMPTY LIST\n");
        return;
    }
    
    printf("===== FILE DESCRIPTOR INFO =====\n");
    t_cmd *current = head;
    int index = 0;
    
    while (current)
    {
        printf("CMD[%d]: fd=%d", index, current->hd);
        
        if (current->hd == -1)
            printf(" (INVALID/CLOSED)");
        else if (current->hd == 0)
            printf(" (STDIN)");
        else if (current->hd == 1)
            printf(" (STDOUT)");
        else if (current->hd == 2)
            printf(" (STDERR)");
        else if (current->hd > 2)
            printf(" (CUSTOM FD - likely heredoc)");
        
        printf("\n");
        current = current->next;
        index++;
    }
    printf("===============================\n");
}