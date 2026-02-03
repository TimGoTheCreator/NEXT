# Configuration file for readthedocs.

project = 'NEXT'
author = 'Timofey Zakharchuk'
release = '2026.02.03'

extensions = [
    "myst_parser", 
]

source_suffix = {
    '.rst': 'restructuredText',
    '.md': 'markdown', 
}

templates_path = ['_templates']
exclude_patterns = []

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
