# Configuration file for the Sphinx documentation builder.

project = 'NEXT'
author = 'Timofey Zakharchuk'
release = '2026.02.03'

extensions = [
    'sphinx.ext.autodoc',
    'sphinx.ext.napoleon',
    'sphinx.ext.mathjax',
]

templates_path = ['_templates']
exclude_patterns = []

html_theme = 'sphinx_rtd_theme'
html_static_path = ['_static']
