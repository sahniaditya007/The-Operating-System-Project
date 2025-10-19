"""Windows 1 style interactive demo using pygame.

Run as: python -m demo.main
Or via scons demo (when SCons calls it)
"""
import sys
import os
import pygame
from pygame import Rect
from PIL import Image, ImageFont, ImageDraw


def load_font(size=12):
    try:
        return pygame.font.SysFont('Arial', size)
    except Exception:
        return pygame.font.Font(None, size)


class Window:
    def __init__(self, rect, title, color=(200, 200, 200)):
        self.rect = Rect(rect)
        self.title = title
        self.color = color
        self.dragging = False

    def draw(self, surf, font):
        pygame.draw.rect(surf, (40, 40, 120), (self.rect.x, self.rect.y, self.rect.w, 20))
        pygame.draw.rect(surf, self.color, self.rect)
        title_s = font.render(self.title, True, (255, 255, 255))
        surf.blit(title_s, (self.rect.x + 4, self.rect.y + 2))


def main():
    pygame.init()
    size = (640, 400)
    screen = pygame.display.set_mode(size)
    pygame.display.set_caption('Windows 1 Demo')
    clock = pygame.time.Clock()
    font = load_font(14)

    # Desktop background (simple pattern)
    bg = pygame.Surface(size)
    bg.fill((0, 120, 192))
    for y in range(0, size[1], 16):
        pygame.draw.line(bg, (0, 100, 170), (0, y), (size[0], y), 1)

    windows = [
        Window((60, 60, 220, 140), 'Program Manager'),
        Window((300, 90, 220, 160), 'MS Paint')
    ]

    start_rect = Rect(0, size[1] - 24, 64, 24)
    show_start = False

    dragging_win = None

    running = True
    while running:
        for ev in pygame.event.get():
            if ev.type == pygame.QUIT:
                running = False
            elif ev.type == pygame.MOUSEBUTTONDOWN:
                mx, my = ev.pos
                if start_rect.collidepoint((mx, my)):
                    show_start = not show_start
                else:
                    # Check windows top-down
                    for w in reversed(windows):
                        if w.rect.collidepoint((mx, my)):
                            # bring to front
                            windows.remove(w)
                            windows.append(w)
                            w.dragging = True
                            dragging_win = w
                            dx = mx - w.rect.x
                            dy = my - w.rect.y
                            w._drag_offset = (dx, dy)
                            break
            elif ev.type == pygame.MOUSEBUTTONUP:
                if dragging_win:
                    dragging_win.dragging = False
                    dragging_win = None
            elif ev.type == pygame.MOUSEMOTION:
                if dragging_win and dragging_win.dragging:
                    mx, my = ev.pos
                    dx, dy = dragging_win._drag_offset
                    dragging_win.rect.x = mx - dx
                    dragging_win.rect.y = my - dy

        screen.blit(bg, (0, 0))

        # Draw windows
        for w in windows:
            w.draw(screen, font)

        # Taskbar / Start button
        pygame.draw.rect(screen, (10, 10, 10), (0, size[1] - 24, size[0], 24))
        pygame.draw.rect(screen, (200, 200, 200), start_rect)
        s = font.render('Start', True, (0, 0, 0))
        screen.blit(s, (4, size[1] - 20))

        if show_start:
            menu = Rect(0, size[1] - 120, 140, 96)
            pygame.draw.rect(screen, (220, 220, 220), menu)
            items = ['Programs', 'Settings', 'Shut Down']
            for i, it in enumerate(items):
                txt = font.render(it, True, (0, 0, 0))
                screen.blit(txt, (4, size[1] - 116 + i * 24))

        pygame.display.flip()
        clock.tick(60)

    pygame.quit()


if __name__ == '__main__':
    main()
