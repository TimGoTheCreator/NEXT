#!/usr/bin/env python3
"""
===============================================================================
NEXT Interactive GitHub Hero Banner & Wallpaper Compositor
===============================================================================
Fully customizable banner and wallpaper generator with live CLI controls.

Examples:
  # Quick defaults (3:1 banner, zoom=1.45)
  python make_hero_wallpaper.py nasa_render_snap118.png

  # Adjust zoom level
  python make_hero_wallpaper.py nasa_render_snap118.png --zoom 1.6

  # Change aspect ratio (e.g. 3:1, 4:1, 21:9, 16:9, 2:1)
  python make_hero_wallpaper.py nasa_render_snap118.png --aspect 3:1
  python make_hero_wallpaper.py nasa_render_snap118.png --aspect 4:1
  python make_hero_wallpaper.py nasa_render_snap118.png --aspect 21:9

  # Pan / Shift crop focus (cx, cy as 0.0 - 1.0 percentages)
  python make_hero_wallpaper.py nasa_render_snap118.png --zoom 1.5 --cx 0.50 --cy 0.52

  # Move or resize the "NEXT" title
  python make_hero_wallpaper.py nasa_render_snap118.png --text-y 0.12 --text-size 0.20

  # Export without text (pure galaxy banner)
  python make_hero_wallpaper.py nasa_render_snap118.png --no-text -o pure_galaxy.png
"""

import os
import sys
import argparse
import numpy as np
from PIL import Image, ImageDraw, ImageFont


def find_default_font(size):
    """Locates clean sans-serif system fonts on Windows or falls back to default"""
    font_candidates = [
        "C:\\Windows\\Fonts\\segoeuib.ttf",
        "C:\\Windows\\Fonts\\segoeui.ttf",
        "C:\\Windows\\Fonts\\arialbd.ttf",
        "C:\\Windows\\Fonts\\arial.ttf",
        "C:\\Windows\\Fonts\\consola.ttf",
    ]
    for font_path in font_candidates:
        if os.path.exists(font_path):
            try:
                return ImageFont.truetype(font_path, size)
            except Exception:
                pass
    return ImageFont.load_default()


def parse_aspect_ratio(aspect_str, base_width=2560):
    """Parses aspect string like '3:1', '4:1', '21:9', '16:9' into width and height"""
    try:
        parts = aspect_str.replace("/", ":").split(":")
        w_ratio = float(parts[0])
        h_ratio = float(parts[1])
        height = int(base_width * (h_ratio / w_ratio))
        return base_width, height
    except Exception:
        print(f"Warning: Could not parse aspect ratio '{aspect_str}', using 3:1 default.")
        return base_width, int(base_width / 3.0)


def create_custom_hero(
    image_path="nasa_render_snap118.png",
    output_path="hero_banner_3x1.png",
    title="NEXT",
    aspect="3:1",
    zoom=1.45,
    cx_ratio=0.50,
    cy_ratio=0.52,
    text_y_ratio=0.08,
    text_size_ratio=0.17,
    show_text=True,
    show_vignette=True,
    custom_res=None,
    transparent=False
):
    # Locate image
    search_paths = [
        image_path,
        os.path.join(os.getcwd(), image_path),
        os.path.join(os.path.dirname(__file__), image_path),
        os.path.join(os.path.dirname(__file__), "..", "examples", "threewaymerger", image_path),
        os.path.join(os.path.dirname(__file__), "..", image_path),
        os.path.join("C:\\n2\\next\\examples\\threewaymerger", image_path),
        os.path.join("C:\\n2\\next", image_path),
    ]

    found_path = None
    for p in search_paths:
        if os.path.exists(p):
            found_path = os.path.abspath(p)
            break

    if not found_path:
        print(f"Error: Could not find image file '{image_path}'")
        return

    print(f"[Hero Generator] Loading: {found_path}")
    base_img = Image.open(found_path).convert("RGBA")
    w, h = base_img.size

    # Target Dimensions
    if custom_res:
        target_w, target_h = custom_res
    else:
        target_w, target_h = parse_aspect_ratio(aspect, base_width=2560)

    print(f"[Hero Generator] Target Resolution: {target_w}x{target_h} (Aspect: {aspect})")

    # =========================================================================
    # Crop & Zoom Engine
    # =========================================================================
    zoom = max(0.2, zoom)
    crop_w = int(w / zoom)
    crop_h = int(crop_w * (target_h / float(target_w)))

    # Prevent crop exceeding image bounds
    crop_w = min(w, crop_w)
    crop_h = min(h, crop_h)

    center_x = int(w * cx_ratio)
    center_y = int(h * cy_ratio)

    x1 = max(0, min(w - crop_w, center_x - crop_w // 2))
    y1 = max(0, min(h - crop_h, center_y - crop_h // 2))
    x2 = x1 + crop_w
    y2 = y1 + crop_h

    cropped = base_img.crop((x1, y1, x2, y2))
    print(f"[Hero Generator] Crop Box: {crop_w}x{crop_h} at ({x1}, {y1}) (Zoom: {zoom:.2f}x, Focus: cx={cx_ratio:.2f}, cy={cy_ratio:.2f})")

    # Scale to target banner resolution
    banner = cropped.resize((target_w, target_h), Image.Resampling.LANCZOS)

    # =========================================================================
    # Optional Transparency Mode (Black space -> Transparent Alpha)
    # =========================================================================
    if transparent:
        np_img = np.array(banner, dtype=np.float32)
        r = np_img[:, :, 0] / 255.0
        g = np_img[:, :, 1] / 255.0
        b = np_img[:, :, 2] / 255.0
        
        # Max channel intensity
        intensity = np.maximum(np.maximum(r, g), b)
        
        # Smooth alpha cutoff (black space becomes 0 alpha)
        cutoff = 0.035
        alpha = np.clip((intensity - cutoff) / (1.0 - cutoff), 0.0, 1.0)
        # Power curve for smooth glowing edges
        alpha = np.power(alpha, 0.85)
        
        np_img[:, :, 3] = (alpha * 255.0).astype(np.uint8)
        banner = Image.fromarray(np_img.astype(np.uint8), mode="RGBA")
        print("[Hero Generator] Transparency Applied: Black space converted to Alpha channel.")

    # =========================================================================
    # Optional Top Vignette (disabled in transparent mode)
    # =========================================================================
    if show_vignette and show_text and not transparent:
        vignette = Image.new("RGBA", (target_w, target_h), (0, 0, 0, 0))
        v_draw = ImageDraw.Draw(vignette)
        fade_h = int(target_h * 0.40)
        for y in range(fade_h):
            p = 1.0 - (y / fade_h)
            alpha = int(160 * (p ** 1.8))
            v_draw.line([(0, y), (target_w, y)], fill=(2, 2, 6, alpha))
        banner = Image.alpha_composite(banner, vignette)

    # =========================================================================
    # Optional Typography
    # =========================================================================
    if show_text and title:
        font_size = int(target_h * text_size_ratio)
        font = find_default_font(font_size)
        title_spaced = "  ".join(list(title))
        draw = ImageDraw.Draw(banner)
        bbox = draw.textbbox((0, 0), title_spaced, font=font)
        tw = bbox[2] - bbox[0]
        th = bbox[3] - bbox[1]

        pos_x = (target_w - tw) // 2
        pos_y = int(target_h * text_y_ratio)

        # Draw crisp white title with soft drop shadow
        draw.text((pos_x + 3, pos_y + 3), title_spaced, font=font, fill=(0, 0, 0, 230))
        draw.text((pos_x, pos_y), title_spaced, font=font, fill=(255, 255, 255, 255))
        print(f"[Hero Generator] Title rendered: '{title}' at y={pos_y} (Font size: {font_size})")

    # =========================================================================
    # Save Final Image (PNG with RGBA Transparency)
    # =========================================================================
    out_file = os.path.abspath(output_path)
    if transparent:
        banner.save(out_file, format="PNG", optimize=True)
    else:
        banner.convert("RGB").save(out_file, quality=98)
    print("=" * 75)
    print(f"SUCCESS: Saved Hero Image -> {out_file} ({target_w}x{target_h})")
    print("=" * 75)


def main():
    parser = argparse.ArgumentParser(
        description="NEXT Interactive GitHub Hero Banner & Wallpaper Compositor",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""Examples:
  python make_hero_wallpaper.py nasa_render_snap118.png --aspect 21:9 --transparent
  python make_hero_wallpaper.py nasa_render_snap118.png --zoom 1.5 --transparent
"""
    )
    parser.add_argument("image", nargs="?", default="nasa_render_snap118.png", help="Path to input snapshot image")
    parser.add_argument("-o", "--output", default="hero_banner_3x1.png", help="Output file name (default: hero_banner_3x1.png)")
    parser.add_argument("--zoom", type=float, default=1.45, help="Zoom magnification factor (e.g. 0.9, 1.0, 1.3, 1.6)")
    parser.add_argument("--aspect", default="3:1", help="Aspect ratio, e.g. 3:1, 4:1, 21:9, 16:9, 2:1 (default: 3:1)")
    parser.add_argument("--cx", type=float, default=0.50, help="Horizontal center focus 0.0 to 1.0 (default: 0.50)")
    parser.add_argument("--cy", type=float, default=0.52, help="Vertical center focus 0.0 to 1.0 (default: 0.52)")
    parser.add_argument("--title", default="NEXT", help="Title wordmark text (default: 'NEXT')")
    parser.add_argument("--text-y", type=float, default=0.08, help="Vertical position of title 0.0 to 1.0 (default: 0.08)")
    parser.add_argument("--text-size", type=float, default=0.17, help="Text size scaling ratio (default: 0.17)")
    parser.add_argument("--transparent", action="store_true", help="Convert black space into transparent alpha for seamless GitHub UI blending")
    parser.add_argument("--no-text", action="store_true", help="Disable text overlay completely")
    parser.add_argument("--no-vignette", action="store_true", help="Disable top vignette darkening")
    parser.add_argument("--res", nargs=2, type=int, default=None, help="Custom resolution width height (e.g. 2560 853)")

    args = parser.parse_args()
    create_custom_hero(
        image_path=args.image,
        output_path=args.output,
        title=args.title,
        aspect=args.aspect,
        zoom=args.zoom,
        cx_ratio=args.cx,
        cy_ratio=args.cy,
        text_y_ratio=args.text_y,
        text_size_ratio=args.text_size,
        show_text=not args.no_text,
        show_vignette=not args.no_vignette,
        custom_res=args.res,
        transparent=args.transparent
    )


if __name__ == "__main__":
    main()
