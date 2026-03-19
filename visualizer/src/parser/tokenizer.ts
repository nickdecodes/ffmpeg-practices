/**
 * 将 ffmpeg 命令字符串拆分为 token 数组
 * 处理引号内的空格
 */
export function tokenize(command: string): string[] {
  const tokens: string[] = [];
  let current = '';
  let inQuote = false;
  let quoteChar = '';

  // 去掉开头的 ffmpeg
  const trimmed = command.trim().replace(/^ffmpeg\s+/, '');

  for (let i = 0; i < trimmed.length; i++) {
    const ch = trimmed[i];

    if (inQuote) {
      if (ch === quoteChar) {
        inQuote = false;
      } else {
        current += ch;
      }
    } else if (ch === '"' || ch === "'") {
      inQuote = true;
      quoteChar = ch;
    } else if (ch === ' ' || ch === '\t') {
      if (current.length > 0) {
        tokens.push(current);
        current = '';
      }
    } else {
      current += ch;
    }
  }

  if (current.length > 0) {
    tokens.push(current);
  }

  return tokens;
}
