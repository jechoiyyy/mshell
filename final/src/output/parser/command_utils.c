/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   command_utils.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 10:49:06 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 14:49:20 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"

t_cmd	*create_command(t_hd *hd_lst)
{
	t_cmd	*cmd;
	t_hd	*last;

	cmd = malloc(sizeof(t_cmd));
	if (!cmd)
		return (NULL);
	cmd->args = NULL;
	cmd->file = NULL;
	cmd->next = NULL;
	last = hd_lst;
	if (last)
	{
		while (last->next)
			last = last->next;
		cmd->hd = last->fd;
	}
	else
		cmd->hd = -1;
	return (cmd);
}

static int	count_args(char **args)
{
	int	count;

	if (!args)
		return (0);
	count = 0;
	while (args[count])
		count++;
	return (count);
}

void	add_argument(t_cmd *cmd, char *arg)
{
	char	**new_args;
	int		count;
	int		i;

	if (!cmd || !arg)
		return ;
	count = count_args(cmd->args);
	new_args = malloc(sizeof(char *) * (count + 2));
	if (!new_args)
		return ;
	i = -1;
	while (++i < count)
		new_args[i] = cmd->args[i];
	new_args[count] = malloc(ft_strlen(arg) + 1);
	if (!new_args[count])
	{
		free(new_args);
		return ;
	}
	ft_strcpy(new_args[count], arg);
	new_args[count + 1] = NULL;
	free(cmd->args);
	cmd->args = new_args;
}

static void	free_args(char **args)
{
	int	i;

	if (!args)
		return ;
	i = 0;
	while (args[i])
	{
		free(args[i]);
		args[i] = NULL;
		i++;
	}
	free(args);
}

static void free_file_list(t_file *file_head)
{
    t_file *current;
    t_file *next;
    
    current = file_head;
    while (current)
    {
        next = current->next;
        if (current->input_file)
        {
            if (current->input_file->filename)
                free(current->input_file->filename);
            free(current->input_file);
        }
        if (current->output_file)
        {
            if (current->output_file->filename)
                free(current->output_file->filename);
            free(current->output_file);
        }
        free(current);
        current = next;
    }
}

void free_commands(t_cmd *cmd)
{
    t_cmd *current;
    t_cmd *next;
    
    current = cmd;
    while (current)
    {
        next = current->next;
        free_args(current->args);
        if (current->file)
            free_file_list(current->file);
        free(current);
        current = next;
    }
}
