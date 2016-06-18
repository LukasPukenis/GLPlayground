# GLPlayground
Graphics playground

## Preparation
* BSP Maps are not included because they might be copyrighted.
* You need to manually resize the textures to max size(256x256) so it would map correctly. Use `FFmpeg` or `Imagick`

inside texture directory

### Windows
`for /r %f in (*.*) do ffmpeg -i %i -vf scale=256:256 -y %i`

### Linux
`for f in *.*; do ffmpeg -i "$f" -vf scale=256:256 -y "$f"; done`
