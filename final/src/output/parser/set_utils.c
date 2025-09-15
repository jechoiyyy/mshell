/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   set_utils.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/27 11:01:35 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 00:56:18 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"

t_filename	*create_filename(void)
{
	t_filename	*new;

	new = malloc(sizeof(t_filename));
	if (!new)
		return (NULL);
	new->filename = NULL;
	new->flag = 0;
	new->append_mode = -1;
	new->hd = -1;
	new->next = NULL;
	return (new);
}

static void	add_filename_to_list(t_filename **head, t_filename *new_node)
{
	t_filename	*current;

	if (!head || !new_node)
		return ;
	if ((*head)->filename == NULL)
	{
		*head = new_node;
		return ;
	}
	current = *head;
	while (current->next)
		current = current->next;
	current->next = new_node;
}

void	set_input_file(t_cmd *cmd, t_token *current,\
	t_prompt *prompt, t_token_type	redir_type)
{
	t_filename	*new_file;
	t_hd		*hd_list;

	hd_list = prompt->hd_lst;
	if (!cmd || !current || !current->value)
		return ;
	new_file = create_filename();
	if (hd_list && hd_list->fd > 0 && redir_type == T_HEREDOC)
	{
		new_file->hd = hd_list->fd;
		prompt->hd_lst = prompt->hd_lst->next;
	}
	else
		new_file->hd = -1;
	new_file->filename = ft_strdup(current->value);
	if (!new_file)
		return ;
	if (current->type == T_WRONG_FILNAME)
		new_file->flag = 1;
	new_file->append_mode = -1;
	add_filename_to_list(&(cmd->input_file), new_file);
}

void	set_output_file(t_cmd *cmd, t_token *current, int append)
{
	t_filename	*new_file;

	if (!cmd || !current || !current->value)
		return ;
	new_file = create_filename();
	new_file->filename = ft_strdup(current->value);
	if (!new_file)
		return ;
	if (current->type == T_WRONG_FILNAME)
		new_file->flag = 1;
	new_file->append_mode = append;
	new_file->hd = -1;
	add_filename_to_list(&(cmd->output_file), new_file);
}

// void	set_heredoc_delimiter(t_cmd *cmd, char *delimiter)
// {
// 	if (!cmd || !delimiter)
// 		return ;
// 	if (cmd->heredoc_delimiter)
// 		free(cmd->heredoc_delimiter);
// 	cmd->heredoc_delimiter = malloc(ft_strlen(delimiter) + 1);
// 	if (!cmd->heredoc_delimiter)
// 		return ;
// 	ft_strcpy(cmd->heredoc_delimiter, delimiter);
// }