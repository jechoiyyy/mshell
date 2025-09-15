/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse_command.c                                    :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jechoi <jechoi@student.42gyeongsan.kr>     +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/26 21:43:39 by jechoi            #+#    #+#             */
/*   Updated: 2025/09/16 01:03:10 by jechoi           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "parser.h"

t_cmd	*parse_simple_command(t_token **current, t_prompt *prompt)
{
	t_cmd	*cmd;

	if (!current || !*current)
		return (NULL);
	cmd = create_command(prompt->hd_lst);
	if (!cmd)
		return (NULL);
	while (*current && (*current)->type != T_PIPE)
	{
		if (is_redirect_token((*current)->type))
		{
			if (parse_redirections(current, cmd, prompt) == FAILURE)
				return (free_commands(cmd), NULL);
		}
		else if ((*current)->type == T_WORD)
		{
			add_argument(cmd, (*current)->value);
			*current = (*current)->next;
		}
		else
			break ;
	}
	return (cmd);
}

int	parse_redirections(t_token **current, t_cmd *cmd, t_prompt *prompt)
{
	t_token_type	redir_type;

	if (!current || !*current || !cmd)
		return (FAILURE);
	redir_type = (*current)->type;
	*current = (*current)->next;
	if (!*current || ((*current)->type != T_WORD
	&& (*current)->type != T_CORRECT_FILNAME
	&& (*current)->type != T_WRONG_FILNAME))
	{
		return (FAILURE);
	}
	if (redir_type == T_REDIR_IN || redir_type == T_HEREDOC)
		set_input_file(cmd, *current, prompt, redir_type);
	else if (redir_type == T_REDIR_OUT)
		set_output_file(cmd, *current, 0);
	else if (redir_type == T_APPEND)
		set_output_file(cmd, *current, 1);
	*current = (*current)->next;
	return (SUCCESS);
}
