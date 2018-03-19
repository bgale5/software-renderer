bool clipTest(double p, double q, double* t1, double* t2)
{
	double r = q/p;

	if(p < 0)
	{
		if(r > *t2) return false;
		else if(r > *t1) *t1 = r;
	}
	else if(p > 0)
	{
		if(r < *t1) return false;
		else if(r < *t2) *t2 = r;
	}
	else if(q < 0) return false;

	return true;
}

void drawLineClipped(BYTE *pFrame, RECT r, POINT2D p1, POINT2D p2)
{
	double t1 = 0.0,
		   t2 = 1.0,
		   dx = p2.x - p1.x,
		   dy;

	int maxX = MAX4(r.p1.x,r.p2.x,r.p3.x,r.p4.x),
		minX = MIN4(r.p1.x,r.p2.x,r.p3.x,r.p4.x),
		maxY = MAX4(r.p1.y,r.p2.y,r.p3.y,r.p4.y),
		minY = MIN4(r.p1.y,r.p2.y,r.p3.y,r.p4.y);

	if( clipTest(-dx, p1.x-minX, &t1, &t2) )
	{
		if( clipTest(dx, maxX-p1.x, &t1, &t2) )
		{
			dy = p2.y-p1.y;

			if( clipTest(-dy, p1.y-minY, &t1, &t2) )
			{
				if( clipTest(dy, maxY-p1.y, &t1, &t2) )
				{
					if(t2 < 1)
					{
						p2.x = p1.x + t2*dx;
						p2.y = p1.y + t2*dy;
					}
					if(t1 > 0)
					{
						p1.x += t1*dx;
						p1.y += t1*dy;
					}
					drawLine(pFrame, p1, p2);
				}
			}
		}
	}
}
