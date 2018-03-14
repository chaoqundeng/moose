#pylint: disable=missing-docstring
import uuid

from MooseDocs.base import components
from MooseDocs.extensions import core
from MooseDocs.tree import tokens, html
from MooseDocs.tree.base import Property

def make_extension():
    return FloatExtension()

class Float(tokens.Token):
    PROPERTIES = [tokens.Property('img', default=False, ptype=bool)]

class Caption(tokens.CountToken):
    PROPERTIES = [Property("key", ptype=unicode)]

    def __init__(self, *args, **kwargs):
        tokens.CountToken.__init__(self, *args, **kwargs)

        #pylint: disable=no-member
        if self.key:
            tokens.Shortcut(self.root, key=self.key, link=u'#{}'.format(self.key),
                            content=u'{} {}'.format(self.prefix.title(), self.number))

class ModalLink(tokens.Link):
    PROPERTIES = [Property("title", ptype=tokens.Token, required=True),
                  Property("content", ptype=tokens.Token, required=True),
                  Property("bottom", ptype=bool, default=False)]

class FloatExtension(components.Extension):
    """
    Provides ability to add caption float elements (e.g., figures, table, etc.). This is only a
    base extension. It does not provide tables for example, just the tools to make floats
    in a uniform manner.
    """
    def extend(self, reader, renderer):
        renderer.add(Float, RenderFloat())
        renderer.add(Caption, RenderCaption())
        renderer.add(ModalLink, RenderModalLink())

    def reinit(self):
        tokens.CountToken.COUNTS.clear()

class RenderFloat(components.RenderComponent):
    def createHTML(self, token, parent): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', **token.attributes)
        div['class'] = 'moose-float-div'
        return div

    def createMaterialize(self, token, parent): #pylint: disable=no-self-use
        div = html.Tag(parent, 'div', **token.attributes)
        div['class'] = 'card'
        content = html.Tag(div, 'div')
        if token.img:
            content['class'] = 'card-image'
        else:
            content['class'] = 'card-content'

        return content

    def createLatex(self, token, parent):
        pass

class RenderCaption(components.RenderComponent):

    def createHTML(self, token, parent): #pylint: disable=no-self-use
        caption = html.Tag(parent, 'p', class_="moose-caption")

        if token.prefix:
            heading = html.Tag(caption, 'span', class_="moose-caption-heading")
            html.String(heading, content=u"{} {}: ".format(token.prefix, token.number))

        text = html.Tag(caption, 'span', class_="moose-caption-text")
        return text

    def createLatex(self, token, parent):
        pass

class RenderModalLink(core.RenderLink):


    def createMaterialize(self, token, parent):
        link = core.RenderLink.createMaterialize(self, token, parent)

        tag = uuid.uuid4()
        link['class'] = 'modal-trigger'
        link['href'] = u'#{}'.format(tag)

        cls = "modal bottom-sheet" if token.bottom else "modal"

        body = parent.root.find('body')
        modal = html.Tag(body, 'div', class_=cls, id_=tag)
        modal_content = html.Tag(modal, 'div', class_="modal-content")

        title = html.Tag(modal_content, 'h4')
        self.translator.renderer.process(title, token.title)

        self.translator.renderer.process(modal_content, token.content)

        return link
